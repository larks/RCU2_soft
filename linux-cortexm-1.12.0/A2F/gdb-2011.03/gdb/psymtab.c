/* Partial symbol tables.
   
   Copyright (C) 2009, 2010 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "symtab.h"
#include "psympriv.h"
#include "objfiles.h"
#include "gdb_assert.h"
#include "block.h"
#include "filenames.h"
#include "source.h"
#include "addrmap.h"
#include "gdbtypes.h"
#include "bcache.h"
#include "ui-out.h"
#include "command.h"
#include "readline/readline.h"
#include "gdb_regex.h"

#ifndef DEV_TTY
#define DEV_TTY "/dev/tty"
#endif

struct psymbol_bcache
{
  struct bcache *bcache;
};

/* A fast way to get from a psymtab to its symtab (after the first time).  */
#define PSYMTAB_TO_SYMTAB(pst)  \
    ((pst) -> symtab != NULL ? (pst) -> symtab : psymtab_to_symtab (pst))

/* Lookup a partial symbol.  */
static struct partial_symbol *lookup_partial_symbol (struct partial_symtab *,
						     const char *, int,
						     domain_enum);

static char *psymtab_to_fullname (struct partial_symtab *ps);

static struct partial_symbol *find_pc_sect_psymbol (struct partial_symtab *,
						    CORE_ADDR,
						    struct obj_section *);

static struct partial_symbol *fixup_psymbol_section (struct partial_symbol
						     *psym,
						     struct objfile *objfile);

static struct symtab *psymtab_to_symtab (struct partial_symtab *pst);

/* Lookup the partial symbol table of a source file named NAME.
   *If* there is no '/' in the name, a match after a '/'
   in the psymtab filename will also work.  */

static struct partial_symtab *
lookup_partial_symtab (struct objfile *objfile, const char *name,
		       const char *full_path, const char *real_path)
{
  struct partial_symtab *pst;

  ALL_OBJFILE_PSYMTABS (objfile, pst)
  {
    if (FILENAME_CMP (name, pst->filename) == 0)
      {
	return (pst);
      }

    /* If the user gave us an absolute path, try to find the file in
       this symtab and use its absolute path.  */
    if (full_path != NULL)
      {
	psymtab_to_fullname (pst);
	if (pst->fullname != NULL
	    && FILENAME_CMP (full_path, pst->fullname) == 0)
	  {
	    return pst;
	  }
      }

    if (real_path != NULL)
      {
        char *rp = NULL;
	psymtab_to_fullname (pst);
        if (pst->fullname != NULL)
          {
            rp = gdb_realpath (pst->fullname);
            make_cleanup (xfree, rp);
          }
	if (rp != NULL && FILENAME_CMP (real_path, rp) == 0)
	  {
	    return pst;
	  }
      }
  }

  /* Now, search for a matching tail (only if name doesn't have any dirs) */

  if (lbasename (name) == name)
    ALL_OBJFILE_PSYMTABS (objfile, pst)
    {
      if (FILENAME_CMP (lbasename (pst->filename), name) == 0)
	return (pst);
    }

  return (NULL);
}

static int
lookup_symtab_via_partial_symtab (struct objfile *objfile, const char *name,
				  const char *full_path, const char *real_path,
				  struct symtab **result)
{
  struct partial_symtab *ps;

  ps = lookup_partial_symtab (objfile, name, full_path, real_path);
  if (!ps)
    return 0;

  if (ps->readin)
    error (_("Internal: readin %s pst for `%s' found when no symtab found."),
	   ps->filename, name);

  *result = PSYMTAB_TO_SYMTAB (ps);
  return 1;
}

/* Find which partial symtab contains PC and SECTION starting at psymtab PST.
   We may find a different psymtab than PST.  See FIND_PC_SECT_PSYMTAB.  */

static struct partial_symtab *
find_pc_sect_psymtab_closer (CORE_ADDR pc, struct obj_section *section,
			     struct partial_symtab *pst,
			     struct minimal_symbol *msymbol)
{
  struct objfile *objfile = pst->objfile;
  struct partial_symtab *tpst;
  struct partial_symtab *best_pst = pst;
  CORE_ADDR best_addr = pst->textlow;

  /* An objfile that has its functions reordered might have
     many partial symbol tables containing the PC, but
     we want the partial symbol table that contains the
     function containing the PC.  */
  if (!(objfile->flags & OBJF_REORDERED) &&
      section == 0)	/* can't validate section this way */
    return pst;

  if (msymbol == NULL)
    return (pst);

  /* The code range of partial symtabs sometimes overlap, so, in
     the loop below, we need to check all partial symtabs and
     find the one that fits better for the given PC address. We
     select the partial symtab that contains a symbol whose
     address is closest to the PC address.  By closest we mean
     that find_pc_sect_symbol returns the symbol with address
     that is closest and still less than the given PC.  */
  for (tpst = pst; tpst != NULL; tpst = tpst->next)
    {
      if (pc >= tpst->textlow && pc < tpst->texthigh)
	{
	  struct partial_symbol *p;
	  CORE_ADDR this_addr;

	  /* NOTE: This assumes that every psymbol has a
	     corresponding msymbol, which is not necessarily
	     true; the debug info might be much richer than the
	     object's symbol table.  */
	  p = find_pc_sect_psymbol (tpst, pc, section);
	  if (p != NULL
	      && SYMBOL_VALUE_ADDRESS (p)
	      == SYMBOL_VALUE_ADDRESS (msymbol))
	    return tpst;

	  /* Also accept the textlow value of a psymtab as a
	     "symbol", to provide some support for partial
	     symbol tables with line information but no debug
	     symbols (e.g. those produced by an assembler).  */
	  if (p != NULL)
	    this_addr = SYMBOL_VALUE_ADDRESS (p);
	  else
	    this_addr = tpst->textlow;

	  /* Check whether it is closer than our current
	     BEST_ADDR.  Since this symbol address is
	     necessarily lower or equal to PC, the symbol closer
	     to PC is the symbol which address is the highest.
	     This way we return the psymtab which contains such
	     best match symbol. This can help in cases where the
	     symbol information/debuginfo is not complete, like
	     for instance on IRIX6 with gcc, where no debug info
	     is emitted for statics. (See also the nodebug.exp
	     testcase.) */
	  if (this_addr > best_addr)
	    {
	      best_addr = this_addr;
	      best_pst = tpst;
	    }
	}
    }
  return best_pst;
}

/* Find which partial symtab contains PC and SECTION.  Return 0 if
   none.  We return the psymtab that contains a symbol whose address
   exactly matches PC, or, if we cannot find an exact match, the
   psymtab that contains a symbol whose address is closest to PC.  */
static struct partial_symtab *
find_pc_sect_psymtab (struct objfile *objfile, CORE_ADDR pc,
		      struct obj_section *section,
		      struct minimal_symbol *msymbol)
{
  struct partial_symtab *pst;

  /* Try just the PSYMTABS_ADDRMAP mapping first as it has better granularity
     than the later used TEXTLOW/TEXTHIGH one.  */

  if (objfile->psymtabs_addrmap != NULL)
    {
      pst = addrmap_find (objfile->psymtabs_addrmap, pc);
      if (pst != NULL)
	{
	  /* FIXME: addrmaps currently do not handle overlayed sections,
	     so fall back to the non-addrmap case if we're debugging
	     overlays and the addrmap returned the wrong section.  */
	  if (overlay_debugging && msymbol && section)
	    {
	      struct partial_symbol *p;

	      /* NOTE: This assumes that every psymbol has a
		 corresponding msymbol, which is not necessarily
		 true; the debug info might be much richer than the
		 object's symbol table.  */
	      p = find_pc_sect_psymbol (pst, pc, section);
	      if (!p
		  || SYMBOL_VALUE_ADDRESS (p)
		  != SYMBOL_VALUE_ADDRESS (msymbol))
		goto next;
	    }

	  /* We do not try to call FIND_PC_SECT_PSYMTAB_CLOSER as
	     PSYMTABS_ADDRMAP we used has already the best 1-byte
	     granularity and FIND_PC_SECT_PSYMTAB_CLOSER may mislead us into
	     a worse chosen section due to the TEXTLOW/TEXTHIGH ranges
	     overlap.  */

	  return pst;
	}
    }

 next:

  /* Existing PSYMTABS_ADDRMAP mapping is present even for PARTIAL_SYMTABs
     which still have no corresponding full SYMTABs read.  But it is not
     present for non-DWARF2 debug infos not supporting PSYMTABS_ADDRMAP in GDB
     so far.  */

  /* Check even OBJFILE with non-zero PSYMTABS_ADDRMAP as only several of
     its CUs may be missing in PSYMTABS_ADDRMAP as they may be varying
     debug info type in single OBJFILE.  */

  ALL_OBJFILE_PSYMTABS (objfile, pst)
    if (pc >= pst->textlow && pc < pst->texthigh)
      {
	struct partial_symtab *best_pst;

	best_pst = find_pc_sect_psymtab_closer (pc, section, pst, msymbol);
	if (best_pst != NULL)
	  return best_pst;
      }

  return NULL;
}

static struct symtab *
find_pc_sect_symtab_from_partial (struct objfile *objfile,
				  struct minimal_symbol *msymbol,
				  CORE_ADDR pc, struct obj_section *section,
				  int warn_if_readin)
{
  struct partial_symtab *ps = find_pc_sect_psymtab (objfile, pc, section,
						    msymbol);
  if (ps)
    {
      if (warn_if_readin && ps->readin)
	/* Might want to error() here (in case symtab is corrupt and
	   will cause a core dump), but maybe we can successfully
	   continue, so let's not.  */
	warning (_("\
(Internal error: pc %s in read in psymtab, but not in symtab.)\n"),
		 paddress (get_objfile_arch (ps->objfile), pc));
      return PSYMTAB_TO_SYMTAB (ps);
    }
  return NULL;
}

/* Find which partial symbol within a psymtab matches PC and SECTION.
   Return 0 if none.  */

static struct partial_symbol *
find_pc_sect_psymbol (struct partial_symtab *psymtab, CORE_ADDR pc,
		      struct obj_section *section)
{
  struct partial_symbol *best = NULL, *p, **pp;
  CORE_ADDR best_pc;

  gdb_assert (psymtab != NULL);

  /* Cope with programs that start at address 0 */
  best_pc = (psymtab->textlow != 0) ? psymtab->textlow - 1 : 0;

  /* Search the global symbols as well as the static symbols, so that
     find_pc_partial_function doesn't use a minimal symbol and thus
     cache a bad endaddr.  */
  for (pp = psymtab->objfile->global_psymbols.list + psymtab->globals_offset;
    (pp - (psymtab->objfile->global_psymbols.list + psymtab->globals_offset)
     < psymtab->n_global_syms);
       pp++)
    {
      p = *pp;
      if (SYMBOL_DOMAIN (p) == VAR_DOMAIN
	  && SYMBOL_CLASS (p) == LOC_BLOCK
	  && pc >= SYMBOL_VALUE_ADDRESS (p)
	  && (SYMBOL_VALUE_ADDRESS (p) > best_pc
	      || (psymtab->textlow == 0
		  && best_pc == 0 && SYMBOL_VALUE_ADDRESS (p) == 0)))
	{
	  if (section)		/* match on a specific section */
	    {
	      fixup_psymbol_section (p, psymtab->objfile);
	      if (!matching_obj_sections (SYMBOL_OBJ_SECTION (p), section))
		continue;
	    }
	  best_pc = SYMBOL_VALUE_ADDRESS (p);
	  best = p;
	}
    }

  for (pp = psymtab->objfile->static_psymbols.list + psymtab->statics_offset;
    (pp - (psymtab->objfile->static_psymbols.list + psymtab->statics_offset)
     < psymtab->n_static_syms);
       pp++)
    {
      p = *pp;
      if (SYMBOL_DOMAIN (p) == VAR_DOMAIN
	  && SYMBOL_CLASS (p) == LOC_BLOCK
	  && pc >= SYMBOL_VALUE_ADDRESS (p)
	  && (SYMBOL_VALUE_ADDRESS (p) > best_pc
	      || (psymtab->textlow == 0
		  && best_pc == 0 && SYMBOL_VALUE_ADDRESS (p) == 0)))
	{
	  if (section)		/* match on a specific section */
	    {
	      fixup_psymbol_section (p, psymtab->objfile);
	      if (!matching_obj_sections (SYMBOL_OBJ_SECTION (p), section))
		continue;
	    }
	  best_pc = SYMBOL_VALUE_ADDRESS (p);
	  best = p;
	}
    }

  return best;
}

static struct partial_symbol *
fixup_psymbol_section (struct partial_symbol *psym, struct objfile *objfile)
{
  CORE_ADDR addr;

  if (!psym)
    return NULL;

  if (SYMBOL_OBJ_SECTION (psym))
    return psym;

  gdb_assert (objfile);

  switch (SYMBOL_CLASS (psym))
    {
    case LOC_STATIC:
    case LOC_LABEL:
    case LOC_BLOCK:
      addr = SYMBOL_VALUE_ADDRESS (psym);
      break;
    default:
      /* Nothing else will be listed in the minsyms -- no use looking
	 it up.  */
      return psym;
    }

  fixup_section (&psym->ginfo, addr, objfile);

  return psym;
}

static struct symtab *
lookup_symbol_aux_psymtabs (struct objfile *objfile,
			    int block_index, const char *name,
			    const domain_enum domain)
{
  struct partial_symtab *ps;
  const int psymtab_index = (block_index == GLOBAL_BLOCK ? 1 : 0);

  ALL_OBJFILE_PSYMTABS (objfile, ps)
  {
    if (!ps->readin && lookup_partial_symbol (ps, name, psymtab_index, domain))
      return PSYMTAB_TO_SYMTAB (ps);
  }

  return NULL;
}

static void
pre_expand_symtabs_matching_psymtabs (struct objfile *objfile,
				      int kind, const char *name,
				      domain_enum domain)
{
  /* Nothing.  */
}

/* Look, in partial_symtab PST, for symbol whose natural name is NAME.
   Check the global symbols if GLOBAL, the static symbols if not. */

static struct partial_symbol *
lookup_partial_symbol (struct partial_symtab *pst, const char *name,
		       int global, domain_enum domain)
{
  struct partial_symbol **start, **psym;
  struct partial_symbol **top, **real_top, **bottom, **center;
  int length = (global ? pst->n_global_syms : pst->n_static_syms);
  int do_linear_search = 1;

  if (length == 0)
    {
      return (NULL);
    }
  start = (global ?
	   pst->objfile->global_psymbols.list + pst->globals_offset :
	   pst->objfile->static_psymbols.list + pst->statics_offset);

  if (global)			/* This means we can use a binary search. */
    {
      do_linear_search = 0;

      /* Binary search.  This search is guaranteed to end with center
         pointing at the earliest partial symbol whose name might be
         correct.  At that point *all* partial symbols with an
         appropriate name will be checked against the correct
         domain.  */

      bottom = start;
      top = start + length - 1;
      real_top = top;
      while (top > bottom)
	{
	  center = bottom + (top - bottom) / 2;
	  if (!(center < top))
	    internal_error (__FILE__, __LINE__, _("failed internal consistency check"));
	  if (!do_linear_search
	      && (SYMBOL_LANGUAGE (*center) == language_java))
	    {
	      do_linear_search = 1;
	    }
	  if (strcmp_iw_ordered (SYMBOL_SEARCH_NAME (*center), name) >= 0)
	    {
	      top = center;
	    }
	  else
	    {
	      bottom = center + 1;
	    }
	}
      if (!(top == bottom))
	internal_error (__FILE__, __LINE__, _("failed internal consistency check"));

      while (top <= real_top
	     && SYMBOL_MATCHES_SEARCH_NAME (*top, name))
	{
	  if (symbol_matches_domain (SYMBOL_LANGUAGE (*top),
				     SYMBOL_DOMAIN (*top), domain))
	    return (*top);
	  top++;
	}
    }

  /* Can't use a binary search or else we found during the binary search that
     we should also do a linear search. */

  if (do_linear_search)
    {
      for (psym = start; psym < start + length; psym++)
	{
	  if (symbol_matches_domain (SYMBOL_LANGUAGE (*psym),
				     SYMBOL_DOMAIN (*psym), domain)
	      && SYMBOL_MATCHES_SEARCH_NAME (*psym, name))
	    return (*psym);
	}
    }

  return (NULL);
}

/* Get the symbol table that corresponds to a partial_symtab.
   This is fast after the first time you do it.  In fact, there
   is an even faster macro PSYMTAB_TO_SYMTAB that does the fast
   case inline.  */

static struct symtab *
psymtab_to_symtab (struct partial_symtab *pst)
{
  /* If it's been looked up before, return it. */
  if (pst->symtab)
    return pst->symtab;

  /* If it has not yet been read in, read it.  */
  if (!pst->readin)
    {
      struct cleanup *back_to = increment_reading_symtab ();

      (*pst->read_symtab) (pst);
      do_cleanups (back_to);
    }

  return pst->symtab;
}

static void
relocate_psymtabs (struct objfile *objfile,
		   struct section_offsets *new_offsets,
		   struct section_offsets *delta)
{
  struct partial_symbol **psym;
  struct partial_symtab *p;

  ALL_OBJFILE_PSYMTABS (objfile, p)
    {
      p->textlow += ANOFFSET (delta, SECT_OFF_TEXT (objfile));
      p->texthigh += ANOFFSET (delta, SECT_OFF_TEXT (objfile));
    }

  for (psym = objfile->global_psymbols.list;
       psym < objfile->global_psymbols.next;
       psym++)
    {
      fixup_psymbol_section (*psym, objfile);
      if (SYMBOL_SECTION (*psym) >= 0)
	SYMBOL_VALUE_ADDRESS (*psym) += ANOFFSET (delta,
						  SYMBOL_SECTION (*psym));
    }
  for (psym = objfile->static_psymbols.list;
       psym < objfile->static_psymbols.next;
       psym++)
    {
      fixup_psymbol_section (*psym, objfile);
      if (SYMBOL_SECTION (*psym) >= 0)
	SYMBOL_VALUE_ADDRESS (*psym) += ANOFFSET (delta,
						  SYMBOL_SECTION (*psym));
    }
}

static struct symtab *
find_last_source_symtab_from_partial (struct objfile *ofp)
{
  struct partial_symtab *ps;
  struct partial_symtab *cs_pst = 0;

  ALL_OBJFILE_PSYMTABS (ofp, ps)
    {
      const char *name = ps->filename;
      int len = strlen (name);

      if (!(len > 2 && (strcmp (&name[len - 2], ".h") == 0
			|| strcmp (name, "<<C++-namespaces>>") == 0)))
	cs_pst = ps;
    }

  if (cs_pst)
    {
      if (cs_pst->readin)
	{
	  internal_error (__FILE__, __LINE__,
			  _("select_source_symtab: "
			  "readin pst found and no symtabs."));
	}
      else
	return PSYMTAB_TO_SYMTAB (cs_pst);
    }
  return NULL;
}

static void
forget_cached_source_info_partial (struct objfile *objfile)
{
  struct partial_symtab *pst;

  ALL_OBJFILE_PSYMTABS (objfile, pst)
    {
      if (pst->fullname != NULL)
	{
	  xfree (pst->fullname);
	  pst->fullname = NULL;
	}
    }
}

static void
print_partial_symbols (struct gdbarch *gdbarch,
		       struct partial_symbol **p, int count, char *what,
		       struct ui_file *outfile)
{
  fprintf_filtered (outfile, "  %s partial symbols:\n", what);
  while (count-- > 0)
    {
      fprintf_filtered (outfile, "    `%s'", SYMBOL_LINKAGE_NAME (*p));
      if (SYMBOL_DEMANGLED_NAME (*p) != NULL)
	{
	  fprintf_filtered (outfile, "  `%s'", SYMBOL_DEMANGLED_NAME (*p));
	}
      fputs_filtered (", ", outfile);
      switch (SYMBOL_DOMAIN (*p))
	{
	case UNDEF_DOMAIN:
	  fputs_filtered ("undefined domain, ", outfile);
	  break;
	case VAR_DOMAIN:
	  /* This is the usual thing -- don't print it */
	  break;
	case STRUCT_DOMAIN:
	  fputs_filtered ("struct domain, ", outfile);
	  break;
	case LABEL_DOMAIN:
	  fputs_filtered ("label domain, ", outfile);
	  break;
	default:
	  fputs_filtered ("<invalid domain>, ", outfile);
	  break;
	}
      switch (SYMBOL_CLASS (*p))
	{
	case LOC_UNDEF:
	  fputs_filtered ("undefined", outfile);
	  break;
	case LOC_CONST:
	  fputs_filtered ("constant int", outfile);
	  break;
	case LOC_STATIC:
	  fputs_filtered ("static", outfile);
	  break;
	case LOC_REGISTER:
	  fputs_filtered ("register", outfile);
	  break;
	case LOC_ARG:
	  fputs_filtered ("pass by value", outfile);
	  break;
	case LOC_REF_ARG:
	  fputs_filtered ("pass by reference", outfile);
	  break;
	case LOC_REGPARM_ADDR:
	  fputs_filtered ("register address parameter", outfile);
	  break;
	case LOC_LOCAL:
	  fputs_filtered ("stack parameter", outfile);
	  break;
	case LOC_TYPEDEF:
	  fputs_filtered ("type", outfile);
	  break;
	case LOC_LABEL:
	  fputs_filtered ("label", outfile);
	  break;
	case LOC_BLOCK:
	  fputs_filtered ("function", outfile);
	  break;
	case LOC_CONST_BYTES:
	  fputs_filtered ("constant bytes", outfile);
	  break;
	case LOC_UNRESOLVED:
	  fputs_filtered ("unresolved", outfile);
	  break;
	case LOC_OPTIMIZED_OUT:
	  fputs_filtered ("optimized out", outfile);
	  break;
	case LOC_COMPUTED:
	  fputs_filtered ("computed at runtime", outfile);
	  break;
	default:
	  fputs_filtered ("<invalid location>", outfile);
	  break;
	}
      fputs_filtered (", ", outfile);
      fputs_filtered (paddress (gdbarch, SYMBOL_VALUE_ADDRESS (*p)), outfile);
      fprintf_filtered (outfile, "\n");
      p++;
    }
}

static void
dump_psymtab (struct objfile *objfile, struct partial_symtab *psymtab,
	      struct ui_file *outfile)
{
  struct gdbarch *gdbarch = get_objfile_arch (objfile);
  int i;

  fprintf_filtered (outfile, "\nPartial symtab for source file %s ",
		    psymtab->filename);
  fprintf_filtered (outfile, "(object ");
  gdb_print_host_address (psymtab, outfile);
  fprintf_filtered (outfile, ")\n\n");
  fprintf_unfiltered (outfile, "  Read from object file %s (",
		      objfile->name);
  gdb_print_host_address (objfile, outfile);
  fprintf_unfiltered (outfile, ")\n");

  if (psymtab->readin)
    {
      fprintf_filtered (outfile,
			"  Full symtab was read (at ");
      gdb_print_host_address (psymtab->symtab, outfile);
      fprintf_filtered (outfile, " by function at ");
      gdb_print_host_address (psymtab->read_symtab, outfile);
      fprintf_filtered (outfile, ")\n");
    }

  fprintf_filtered (outfile, "  Relocate symbols by ");
  for (i = 0; i < psymtab->objfile->num_sections; ++i)
    {
      if (i != 0)
	fprintf_filtered (outfile, ", ");
      wrap_here ("    ");
      fputs_filtered (paddress (gdbarch,
				ANOFFSET (psymtab->section_offsets, i)),
		      outfile);
    }
  fprintf_filtered (outfile, "\n");

  fprintf_filtered (outfile, "  Symbols cover text addresses ");
  fputs_filtered (paddress (gdbarch, psymtab->textlow), outfile);
  fprintf_filtered (outfile, "-");
  fputs_filtered (paddress (gdbarch, psymtab->texthigh), outfile);
  fprintf_filtered (outfile, "\n");
  fprintf_filtered (outfile, "  Depends on %d other partial symtabs.\n",
		    psymtab->number_of_dependencies);
  for (i = 0; i < psymtab->number_of_dependencies; i++)
    {
      fprintf_filtered (outfile, "    %d ", i);
      gdb_print_host_address (psymtab->dependencies[i], outfile);
      fprintf_filtered (outfile, " %s\n",
			psymtab->dependencies[i]->filename);
    }
  if (psymtab->n_global_syms > 0)
    {
      print_partial_symbols (gdbarch,
			     objfile->global_psymbols.list
			     + psymtab->globals_offset,
			     psymtab->n_global_syms, "Global", outfile);
    }
  if (psymtab->n_static_syms > 0)
    {
      print_partial_symbols (gdbarch,
			     objfile->static_psymbols.list
			     + psymtab->statics_offset,
			     psymtab->n_static_syms, "Static", outfile);
    }
  fprintf_filtered (outfile, "\n");
}

static void
print_psymtab_stats_for_objfile (struct objfile *objfile)
{
  int i;
  struct partial_symtab *ps;

  i = 0;
  ALL_OBJFILE_PSYMTABS (objfile, ps)
    {
      if (ps->readin == 0)
	i++;
    }
  printf_filtered (_("  Number of psym tables (not yet expanded): %d\n"), i);
}

static void
dump_psymtabs_for_objfile (struct objfile *objfile)
{
  struct partial_symtab *psymtab;

  if (objfile->psymtabs)
    {
      printf_filtered ("Psymtabs:\n");
      for (psymtab = objfile->psymtabs;
	   psymtab != NULL;
	   psymtab = psymtab->next)
	{
	  printf_filtered ("%s at ",
			   psymtab->filename);
	  gdb_print_host_address (psymtab, gdb_stdout);
	  printf_filtered (", ");
	  if (psymtab->objfile != objfile)
	    {
	      printf_filtered ("NOT ON CHAIN!  ");
	    }
	  wrap_here ("  ");
	}
      printf_filtered ("\n\n");
    }
}

/* Look through the partial symtabs for all symbols which begin
   by matching FUNC_NAME.  Make sure we read that symbol table in. */

static void
read_symtabs_for_function (struct objfile *objfile, const char *func_name)
{
  struct partial_symtab *ps;

  ALL_OBJFILE_PSYMTABS (objfile, ps)
  {
    if (ps->readin)
      continue;

    if ((lookup_partial_symbol (ps, func_name, 1, VAR_DOMAIN)
	 != NULL)
	|| (lookup_partial_symbol (ps, func_name, 0, VAR_DOMAIN)
	    != NULL))
      psymtab_to_symtab (ps);
  }
}

static void
expand_partial_symbol_tables (struct objfile *objfile)
{
  struct partial_symtab *psymtab;

  for (psymtab = objfile->psymtabs;
       psymtab != NULL;
       psymtab = psymtab->next)
    {
      psymtab_to_symtab (psymtab);
    }
}

static void
read_psymtabs_with_filename (struct objfile *objfile, const char *filename)
{
  struct partial_symtab *p;

  ALL_OBJFILE_PSYMTABS (objfile, p)
    {
      if (strcmp (filename, p->filename) == 0)
	PSYMTAB_TO_SYMTAB (p);
    }
}

static void
map_symbol_names_psymtab (struct objfile *objfile,
			  void (*fun) (const char *, void *), void *data)
{
  struct partial_symtab *ps;

  ALL_OBJFILE_PSYMTABS (objfile, ps)
    {
      struct partial_symbol **psym;

      /* If the psymtab's been read in we'll get it when we search
	 through the blockvector.  */
      if (ps->readin)
	continue;

      for (psym = objfile->global_psymbols.list + ps->globals_offset;
	   psym < (objfile->global_psymbols.list + ps->globals_offset
		   + ps->n_global_syms);
	   psym++)
	{
	  /* If interrupted, then quit. */
	  QUIT;
	  (*fun) (SYMBOL_NATURAL_NAME (*psym), data);
	}

      for (psym = objfile->static_psymbols.list + ps->statics_offset;
	   psym < (objfile->static_psymbols.list + ps->statics_offset
		   + ps->n_static_syms);
	   psym++)
	{
	  QUIT;
	  (*fun) (SYMBOL_NATURAL_NAME (*psym), data);
	}
    }
}

static void
map_symbol_filenames_psymtab (struct objfile *objfile,
			      void (*fun) (const char *, const char *,
					   void *),
			      void *data)
{
  struct partial_symtab *ps;

  ALL_OBJFILE_PSYMTABS (objfile, ps)
    {
      const char *fullname;

      if (ps->readin)
	continue;

      fullname = psymtab_to_fullname (ps);
      (*fun) (ps->filename, fullname, data);
    }
}

int find_and_open_source (const char *filename,
			  const char *dirname,
			  char **fullname);

/* Finds the fullname that a partial_symtab represents.

   If this functions finds the fullname, it will save it in ps->fullname
   and it will also return the value.

   If this function fails to find the file that this partial_symtab represents,
   NULL will be returned and ps->fullname will be set to NULL.  */
static char *
psymtab_to_fullname (struct partial_symtab *ps)
{
  int r;

  if (!ps)
    return NULL;

  /* Don't check ps->fullname here, the file could have been
     deleted/moved/..., look for it again */
  r = find_and_open_source (ps->filename, ps->dirname, &ps->fullname);

  if (r >= 0)
    {
      close (r);
      return ps->fullname;
    }

  return NULL;
}

static const char *
find_symbol_file_from_partial (struct objfile *objfile, const char *name)
{
  struct partial_symtab *pst;

  ALL_OBJFILE_PSYMTABS (objfile, pst)
    {
      if (lookup_partial_symbol (pst, name, 1, VAR_DOMAIN))
	return pst->filename;
    }
  return NULL;
}

/* Look, in partial_symtab PST, for symbol NAME in given namespace.
   Check the global symbols if GLOBAL, the static symbols if not.
   Do wild-card match if WILD.  */

static struct partial_symbol *
ada_lookup_partial_symbol (struct partial_symtab *pst, const char *name,
                           int global, domain_enum namespace, int wild,
			   int (*wild_match) (const char *, int, const char *),
			   int (*is_name_suffix) (const char *))
{
  struct partial_symbol **start;
  int name_len = strlen (name);
  int length = (global ? pst->n_global_syms : pst->n_static_syms);
  int i;

  if (length == 0)
    {
      return (NULL);
    }

  start = (global ?
           pst->objfile->global_psymbols.list + pst->globals_offset :
           pst->objfile->static_psymbols.list + pst->statics_offset);

  if (wild)
    {
      for (i = 0; i < length; i += 1)
        {
          struct partial_symbol *psym = start[i];

          if (symbol_matches_domain (SYMBOL_LANGUAGE (psym),
                                     SYMBOL_DOMAIN (psym), namespace)
              && (*wild_match) (name, name_len, SYMBOL_LINKAGE_NAME (psym)))
            return psym;
        }
      return NULL;
    }
  else
    {
      if (global)
        {
          int U;

          i = 0;
          U = length - 1;
          while (U - i > 4)
            {
              int M = (U + i) >> 1;
              struct partial_symbol *psym = start[M];

              if (SYMBOL_LINKAGE_NAME (psym)[0] < name[0])
                i = M + 1;
              else if (SYMBOL_LINKAGE_NAME (psym)[0] > name[0])
                U = M - 1;
              else if (strcmp (SYMBOL_LINKAGE_NAME (psym), name) < 0)
                i = M + 1;
              else
                U = M;
            }
        }
      else
        i = 0;

      while (i < length)
        {
          struct partial_symbol *psym = start[i];

          if (symbol_matches_domain (SYMBOL_LANGUAGE (psym),
                                     SYMBOL_DOMAIN (psym), namespace))
            {
              int cmp = strncmp (name, SYMBOL_LINKAGE_NAME (psym), name_len);

              if (cmp < 0)
                {
                  if (global)
                    break;
                }
              else if (cmp == 0
                       && (*is_name_suffix) (SYMBOL_LINKAGE_NAME (psym)
					     + name_len))
                return psym;
            }
          i += 1;
        }

      if (global)
        {
          int U;

          i = 0;
          U = length - 1;
          while (U - i > 4)
            {
              int M = (U + i) >> 1;
              struct partial_symbol *psym = start[M];

              if (SYMBOL_LINKAGE_NAME (psym)[0] < '_')
                i = M + 1;
              else if (SYMBOL_LINKAGE_NAME (psym)[0] > '_')
                U = M - 1;
              else if (strcmp (SYMBOL_LINKAGE_NAME (psym), "_ada_") < 0)
                i = M + 1;
              else
                U = M;
            }
        }
      else
        i = 0;

      while (i < length)
        {
          struct partial_symbol *psym = start[i];

          if (symbol_matches_domain (SYMBOL_LANGUAGE (psym),
                                     SYMBOL_DOMAIN (psym), namespace))
            {
              int cmp;

              cmp = (int) '_' - (int) SYMBOL_LINKAGE_NAME (psym)[0];
              if (cmp == 0)
                {
                  cmp = strncmp ("_ada_", SYMBOL_LINKAGE_NAME (psym), 5);
                  if (cmp == 0)
                    cmp = strncmp (name, SYMBOL_LINKAGE_NAME (psym) + 5,
                                   name_len);
                }

              if (cmp < 0)
                {
                  if (global)
                    break;
                }
              else if (cmp == 0
                       && (*is_name_suffix) (SYMBOL_LINKAGE_NAME (psym)
					     + name_len + 5))
                return psym;
            }
          i += 1;
        }
    }
  return NULL;
}

static void
map_ada_symtabs (struct objfile *objfile,
		 int (*wild_match) (const char *, int, const char *),
		 int (*is_name_suffix) (const char *),
		 void (*callback) (struct objfile *, struct symtab *, void *),
		 const char *name, int global, domain_enum namespace, int wild,
		 void *data)
{
  struct partial_symtab *ps;

  ALL_OBJFILE_PSYMTABS (objfile, ps)
    {
      QUIT;
      if (ps->readin
	  || ada_lookup_partial_symbol (ps, name, global, namespace, wild,
					wild_match, is_name_suffix))
	{
	  struct symtab *s = PSYMTAB_TO_SYMTAB (ps);

	  if (s == NULL || !s->primary)
	    continue;
	  (*callback) (objfile, s, data);
	}
    }
}

static void
expand_symtabs_matching_via_partial (struct objfile *objfile,
				     int (*file_matcher) (const char *, void *),
				     int (*name_matcher) (const char *, void *),
				     domain_enum kind,
				     void *data)
{
  struct partial_symtab *ps;

  ALL_OBJFILE_PSYMTABS (objfile, ps)
    {
      struct partial_symbol **psym;
      struct partial_symbol **bound, **gbound, **sbound;
      int keep_going = 1;

      if (ps->readin)
	continue;

      if (! (*file_matcher) (ps->filename, data))
	continue;

      gbound = objfile->global_psymbols.list + ps->globals_offset + ps->n_global_syms;
      sbound = objfile->static_psymbols.list + ps->statics_offset + ps->n_static_syms;
      bound = gbound;

      /* Go through all of the symbols stored in a partial
	 symtab in one loop. */
      psym = objfile->global_psymbols.list + ps->globals_offset;
      while (keep_going)
	{
	  if (psym >= bound)
	    {
	      if (bound == gbound && ps->n_static_syms != 0)
		{
		  psym = objfile->static_psymbols.list + ps->statics_offset;
		  bound = sbound;
		}
	      else
		keep_going = 0;
	      continue;
	    }
	  else
	    {
	      QUIT;

	      if ((*name_matcher) (SYMBOL_NATURAL_NAME (*psym), data)
		  && ((kind == VARIABLES_DOMAIN
		       && SYMBOL_CLASS (*psym) != LOC_TYPEDEF
		       && SYMBOL_CLASS (*psym) != LOC_BLOCK)
		      || (kind == FUNCTIONS_DOMAIN
			  && SYMBOL_CLASS (*psym) == LOC_BLOCK)
		      || (kind == TYPES_DOMAIN
			  && SYMBOL_CLASS (*psym) == LOC_TYPEDEF)))
		{
		  PSYMTAB_TO_SYMTAB (ps);
		  keep_going = 0;
		}
	    }
	  psym++;
	}
    }
}

static int
objfile_has_psyms (struct objfile *objfile)
{
  return objfile->psymtabs != NULL;
}

const struct quick_symbol_functions psym_functions =
{
  objfile_has_psyms,
  find_last_source_symtab_from_partial,
  forget_cached_source_info_partial,
  lookup_symtab_via_partial_symtab,
  lookup_symbol_aux_psymtabs,
  pre_expand_symtabs_matching_psymtabs,
  print_psymtab_stats_for_objfile,
  dump_psymtabs_for_objfile,
  relocate_psymtabs,
  read_symtabs_for_function,
  expand_partial_symbol_tables,
  read_psymtabs_with_filename,
  find_symbol_file_from_partial,
  map_ada_symtabs,
  expand_symtabs_matching_via_partial,
  find_pc_sect_symtab_from_partial,
  map_symbol_names_psymtab,
  map_symbol_filenames_psymtab
};



/* This compares two partial symbols by names, using strcmp_iw_ordered
   for the comparison.  */

static int
compare_psymbols (const void *s1p, const void *s2p)
{
  struct partial_symbol *const *s1 = s1p;
  struct partial_symbol *const *s2 = s2p;

  return strcmp_iw_ordered (SYMBOL_SEARCH_NAME (*s1),
			    SYMBOL_SEARCH_NAME (*s2));
}

void
sort_pst_symbols (struct partial_symtab *pst)
{
  /* Sort the global list; don't sort the static list */

  qsort (pst->objfile->global_psymbols.list + pst->globals_offset,
	 pst->n_global_syms, sizeof (struct partial_symbol *),
	 compare_psymbols);
}

/* Allocate and partially fill a partial symtab.  It will be
   completely filled at the end of the symbol list.

   FILENAME is the name of the symbol-file we are reading from. */

struct partial_symtab *
start_psymtab_common (struct objfile *objfile,
		      struct section_offsets *section_offsets,
		      const char *filename,
		      CORE_ADDR textlow, struct partial_symbol **global_syms,
		      struct partial_symbol **static_syms)
{
  struct partial_symtab *psymtab;

  psymtab = allocate_psymtab (filename, objfile);
  psymtab->section_offsets = section_offsets;
  psymtab->textlow = textlow;
  psymtab->texthigh = psymtab->textlow;		/* default */
  psymtab->globals_offset = global_syms - objfile->global_psymbols.list;
  psymtab->statics_offset = static_syms - objfile->static_psymbols.list;
  return (psymtab);
}

/* Calculate a hash code for the given partial symbol.  The hash is
   calculated using the symbol's value, language, domain, class
   and name. These are the values which are set by
   add_psymbol_to_bcache.  */

static unsigned long
psymbol_hash (const void *addr, int length)
{
  unsigned long h = 0;
  struct partial_symbol *psymbol = (struct partial_symbol *) addr;
  unsigned int lang = psymbol->ginfo.language;
  unsigned int domain = PSYMBOL_DOMAIN (psymbol);
  unsigned int class = PSYMBOL_CLASS (psymbol);

  h = hash_continue (&psymbol->ginfo.value, sizeof (psymbol->ginfo.value), h);
  h = hash_continue (&lang, sizeof (unsigned int), h);
  h = hash_continue (&domain, sizeof (unsigned int), h);
  h = hash_continue (&class, sizeof (unsigned int), h);
  h = hash_continue (psymbol->ginfo.name, strlen (psymbol->ginfo.name), h);

  return h;
}

/* Returns true if the symbol at addr1 equals the symbol at addr2.
   For the comparison this function uses a symbols value,
   language, domain, class and name.  */

static int
psymbol_compare (const void *addr1, const void *addr2, int length)
{
  struct partial_symbol *sym1 = (struct partial_symbol *) addr1;
  struct partial_symbol *sym2 = (struct partial_symbol *) addr2;

  return (memcmp (&sym1->ginfo.value, &sym1->ginfo.value,
                  sizeof (sym1->ginfo.value)) == 0
	  && sym1->ginfo.language == sym2->ginfo.language
          && PSYMBOL_DOMAIN (sym1) == PSYMBOL_DOMAIN (sym2)
          && PSYMBOL_CLASS (sym1) == PSYMBOL_CLASS (sym2)
          && sym1->ginfo.name == sym2->ginfo.name);
}

/* Initialize a partial symbol bcache.  */

struct psymbol_bcache *
psymbol_bcache_init (void)
{
  struct psymbol_bcache *bcache = XCALLOC (1, struct psymbol_bcache);
  bcache->bcache = bcache_xmalloc (psymbol_hash, psymbol_compare);
  return bcache;
}

/* Free a partial symbol bcache.  */
void
psymbol_bcache_free (struct psymbol_bcache *bcache)
{
  if (bcache == NULL)
    return;

  bcache_xfree (bcache->bcache);
  xfree (bcache);
}

/* Return the internal bcache of the psymbol_bcache BCACHE*/

struct bcache *
psymbol_bcache_get_bcache (struct psymbol_bcache *bcache)
{
  return bcache->bcache;
}

/* Find a copy of the SYM in BCACHE.  If BCACHE has never seen this
   symbol before, add a copy to BCACHE.  In either case, return a pointer
   to BCACHE's copy of the symbol.  If optional ADDED is not NULL, return
   1 in case of new entry or 0 if returning an old entry.  */

static const struct partial_symbol *
psymbol_bcache_full (struct partial_symbol *sym,
                     struct psymbol_bcache *bcache,
                     int *added)
{
  return bcache_full (sym,
                      sizeof (struct partial_symbol),
                      bcache->bcache,
                      added);
}

/* Helper function, initialises partial symbol structure and stashes 
   it into objfile's bcache.  Note that our caching mechanism will
   use all fields of struct partial_symbol to determine hash value of the
   structure.  In other words, having two symbols with the same name but
   different domain (or address) is possible and correct.  */

static const struct partial_symbol *
add_psymbol_to_bcache (char *name, int namelength, int copy_name,
		       domain_enum domain,
		       enum address_class class,
		       long val,	/* Value as a long */
		       CORE_ADDR coreaddr,	/* Value as a CORE_ADDR */
		       enum language language, struct objfile *objfile,
		       int *added)
{
  struct partial_symbol psymbol;

  /* We must ensure that the entire 'value' field has been zeroed
     before assigning to it, because an assignment may not write the
     entire field.  */
  memset (&psymbol.ginfo.value, 0, sizeof (psymbol.ginfo.value));

  /* val and coreaddr are mutually exclusive, one of them *will* be zero */
  if (val != 0)
    {
      SYMBOL_VALUE (&psymbol) = val;
    }
  else
    {
      SYMBOL_VALUE_ADDRESS (&psymbol) = coreaddr;
    }
  SYMBOL_SECTION (&psymbol) = 0;
  SYMBOL_OBJ_SECTION (&psymbol) = NULL;
  SYMBOL_SET_LANGUAGE (&psymbol, language);
  PSYMBOL_DOMAIN (&psymbol) = domain;
  PSYMBOL_CLASS (&psymbol) = class;

  SYMBOL_SET_NAMES (&psymbol, name, namelength, copy_name, objfile);

  /* Stash the partial symbol away in the cache */
  return psymbol_bcache_full (&psymbol,
                              objfile->psymbol_cache,
                              added);
}

/* Helper function, adds partial symbol to the given partial symbol
   list.  */

static void
append_psymbol_to_list (struct psymbol_allocation_list *list,
			const struct partial_symbol *psym,
			struct objfile *objfile)
{
  if (list->next >= list->list + list->size)
    extend_psymbol_list (list, objfile);
  *list->next++ = (struct partial_symbol *) psym;
  OBJSTAT (objfile, n_psyms++);
}

/* Add a symbol with a long value to a psymtab.
   Since one arg is a struct, we pass in a ptr and deref it (sigh).
   Return the partial symbol that has been added.  */

/* NOTE: carlton/2003-09-11: The reason why we return the partial
   symbol is so that callers can get access to the symbol's demangled
   name, which they don't have any cheap way to determine otherwise.
   (Currenly, dwarf2read.c is the only file who uses that information,
   though it's possible that other readers might in the future.)
   Elena wasn't thrilled about that, and I don't blame her, but we
   couldn't come up with a better way to get that information.  If
   it's needed in other situations, we could consider breaking up
   SYMBOL_SET_NAMES to provide access to the demangled name lookup
   cache.  */

const struct partial_symbol *
add_psymbol_to_list (char *name, int namelength, int copy_name,
		     domain_enum domain,
		     enum address_class class,
		     struct psymbol_allocation_list *list, 
		     long val,	/* Value as a long */
		     CORE_ADDR coreaddr,	/* Value as a CORE_ADDR */
		     enum language language, struct objfile *objfile)
{
  const struct partial_symbol *psym;

  int added;

  /* Stash the partial symbol away in the cache */
  psym = add_psymbol_to_bcache (name, namelength, copy_name, domain, class,
				val, coreaddr, language, objfile, &added);

  /* Do not duplicate global partial symbols.  */
  if (list == &objfile->global_psymbols
      && !added)
    return psym;

  /* Save pointer to partial symbol in psymtab, growing symtab if needed. */
  append_psymbol_to_list (list, psym, objfile);
  return psym;
}

/* Initialize storage for partial symbols.  */

void
init_psymbol_list (struct objfile *objfile, int total_symbols)
{
  /* Free any previously allocated psymbol lists.  */

  if (objfile->global_psymbols.list)
    {
      xfree (objfile->global_psymbols.list);
    }
  if (objfile->static_psymbols.list)
    {
      xfree (objfile->static_psymbols.list);
    }

  /* Current best guess is that approximately a twentieth
     of the total symbols (in a debugging file) are global or static
     oriented symbols */

  objfile->global_psymbols.size = total_symbols / 10;
  objfile->static_psymbols.size = total_symbols / 10;

  if (objfile->global_psymbols.size > 0)
    {
      objfile->global_psymbols.next =
	objfile->global_psymbols.list = (struct partial_symbol **)
	xmalloc ((objfile->global_psymbols.size
		  * sizeof (struct partial_symbol *)));
    }
  if (objfile->static_psymbols.size > 0)
    {
      objfile->static_psymbols.next =
	objfile->static_psymbols.list = (struct partial_symbol **)
	xmalloc ((objfile->static_psymbols.size
		  * sizeof (struct partial_symbol *)));
    }
}

struct partial_symtab *
allocate_psymtab (const char *filename, struct objfile *objfile)
{
  struct partial_symtab *psymtab;

  if (objfile->free_psymtabs)
    {
      psymtab = objfile->free_psymtabs;
      objfile->free_psymtabs = psymtab->next;
    }
  else
    psymtab = (struct partial_symtab *)
      obstack_alloc (&objfile->objfile_obstack,
		     sizeof (struct partial_symtab));

  memset (psymtab, 0, sizeof (struct partial_symtab));
  psymtab->filename = obsavestring (filename, strlen (filename),
				    &objfile->objfile_obstack);
  psymtab->symtab = NULL;

  /* Prepend it to the psymtab list for the objfile it belongs to.
     Psymtabs are searched in most recent inserted -> least recent
     inserted order. */

  psymtab->objfile = objfile;
  psymtab->next = objfile->psymtabs;
  objfile->psymtabs = psymtab;

  return (psymtab);
}

void
discard_psymtab (struct partial_symtab *pst)
{
  struct partial_symtab **prev_pst;

  /* From dbxread.c:
     Empty psymtabs happen as a result of header files which don't
     have any symbols in them.  There can be a lot of them.  But this
     check is wrong, in that a psymtab with N_SLINE entries but
     nothing else is not empty, but we don't realize that.  Fixing
     that without slowing things down might be tricky.  */

  /* First, snip it out of the psymtab chain */

  prev_pst = &(pst->objfile->psymtabs);
  while ((*prev_pst) != pst)
    prev_pst = &((*prev_pst)->next);
  (*prev_pst) = pst->next;

  /* Next, put it on a free list for recycling */

  pst->next = pst->objfile->free_psymtabs;
  pst->objfile->free_psymtabs = pst;
}

/* Increase the space allocated for LISTP, which is probably
   global_psymbols or static_psymbols. This space will eventually
   be freed in free_objfile().  */

void
extend_psymbol_list (struct psymbol_allocation_list *listp,
		     struct objfile *objfile)
{
  int new_size;

  if (listp->size == 0)
    {
      new_size = 255;
      listp->list = (struct partial_symbol **)
	xmalloc (new_size * sizeof (struct partial_symbol *));
    }
  else
    {
      new_size = listp->size * 2;
      listp->list = (struct partial_symbol **)
	xrealloc ((char *) listp->list,
		  new_size * sizeof (struct partial_symbol *));
    }
  /* Next assumes we only went one over.  Should be good if
     program works correctly */
  listp->next = listp->list + listp->size;
  listp->size = new_size;
}



void
maintenance_print_psymbols (char *args, int from_tty)
{
  char **argv;
  struct ui_file *outfile;
  struct cleanup *cleanups;
  char *symname = NULL;
  char *filename = DEV_TTY;
  struct objfile *objfile;
  struct partial_symtab *ps;

  dont_repeat ();

  if (args == NULL)
    {
      error (_("print-psymbols takes an output file name and optional symbol file name"));
    }
  argv = gdb_buildargv (args);
  cleanups = make_cleanup_freeargv (argv);

  if (argv[0] != NULL)
    {
      filename = argv[0];
      /* If a second arg is supplied, it is a source file name to match on */
      if (argv[1] != NULL)
	{
	  symname = argv[1];
	}
    }

  filename = tilde_expand (filename);
  make_cleanup (xfree, filename);

  outfile = gdb_fopen (filename, FOPEN_WT);
  if (outfile == 0)
    perror_with_name (filename);
  make_cleanup_ui_file_delete (outfile);

  immediate_quit++;
  ALL_PSYMTABS (objfile, ps)
    if (symname == NULL || strcmp (symname, ps->filename) == 0)
    dump_psymtab (objfile, ps, outfile);
  immediate_quit--;
  do_cleanups (cleanups);
}

/* List all the partial symbol tables whose names match REGEXP (optional).  */
void
maintenance_info_psymtabs (char *regexp, int from_tty)
{
  struct program_space *pspace;
  struct objfile *objfile;

  if (regexp)
    re_comp (regexp);

  ALL_PSPACES (pspace)
    ALL_PSPACE_OBJFILES (pspace, objfile)
    {
      struct gdbarch *gdbarch = get_objfile_arch (objfile);
      struct partial_symtab *psymtab;

      /* We don't want to print anything for this objfile until we
         actually find a symtab whose name matches.  */
      int printed_objfile_start = 0;

      ALL_OBJFILE_PSYMTABS (objfile, psymtab)
	{
	  QUIT;

	  if (! regexp
	      || re_exec (psymtab->filename))
	    {
	      if (! printed_objfile_start)
		{
		  printf_filtered ("{ objfile %s ", objfile->name);
		  wrap_here ("  ");
		  printf_filtered ("((struct objfile *) %s)\n", 
				   host_address_to_string (objfile));
		  printed_objfile_start = 1;
		}

	      printf_filtered ("  { psymtab %s ", psymtab->filename);
	      wrap_here ("    ");
	      printf_filtered ("((struct partial_symtab *) %s)\n", 
			       host_address_to_string (psymtab));

	      printf_filtered ("    readin %s\n",
			       psymtab->readin ? "yes" : "no");
	      printf_filtered ("    fullname %s\n",
			       psymtab->fullname ? psymtab->fullname : "(null)");
	      printf_filtered ("    text addresses ");
	      fputs_filtered (paddress (gdbarch, psymtab->textlow),
			      gdb_stdout);
	      printf_filtered (" -- ");
	      fputs_filtered (paddress (gdbarch, psymtab->texthigh),
			      gdb_stdout);
	      printf_filtered ("\n");
	      printf_filtered ("    globals ");
	      if (psymtab->n_global_syms)
		{
		  printf_filtered ("(* (struct partial_symbol **) %s @ %d)\n",
				   host_address_to_string (psymtab->objfile->global_psymbols.list
				    + psymtab->globals_offset),
				   psymtab->n_global_syms);
		}
	      else
		printf_filtered ("(none)\n");
	      printf_filtered ("    statics ");
	      if (psymtab->n_static_syms)
		{
		  printf_filtered ("(* (struct partial_symbol **) %s @ %d)\n",
				   host_address_to_string (psymtab->objfile->static_psymbols.list
				    + psymtab->statics_offset),
				   psymtab->n_static_syms);
		}
	      else
		printf_filtered ("(none)\n");
	      printf_filtered ("    dependencies ");
	      if (psymtab->number_of_dependencies)
		{
		  int i;

		  printf_filtered ("{\n");
		  for (i = 0; i < psymtab->number_of_dependencies; i++)
		    {
		      struct partial_symtab *dep = psymtab->dependencies[i];

		      /* Note the string concatenation there --- no comma.  */
		      printf_filtered ("      psymtab %s "
				       "((struct partial_symtab *) %s)\n",
				       dep->filename, 
				       host_address_to_string (dep));
		    }
		  printf_filtered ("    }\n");
		}
	      else
		printf_filtered ("(none)\n");
	      printf_filtered ("  }\n");
	    }
	}

      if (printed_objfile_start)
        printf_filtered ("}\n");
    }
}

/* Check consistency of psymtabs and symtabs.  */

void
maintenance_check_symtabs (char *ignore, int from_tty)
{
  struct symbol *sym;
  struct partial_symbol **psym;
  struct symtab *s = NULL;
  struct partial_symtab *ps;
  struct blockvector *bv;
  struct objfile *objfile;
  struct block *b;
  int length;

  ALL_PSYMTABS (objfile, ps)
  {
    struct gdbarch *gdbarch = get_objfile_arch (objfile);

    s = PSYMTAB_TO_SYMTAB (ps);
    if (s == NULL)
      continue;
    bv = BLOCKVECTOR (s);
    b = BLOCKVECTOR_BLOCK (bv, STATIC_BLOCK);
    psym = ps->objfile->static_psymbols.list + ps->statics_offset;
    length = ps->n_static_syms;
    while (length--)
      {
	sym = lookup_block_symbol (b, SYMBOL_LINKAGE_NAME (*psym),
				   SYMBOL_DOMAIN (*psym));
	if (!sym)
	  {
	    printf_filtered ("Static symbol `");
	    puts_filtered (SYMBOL_LINKAGE_NAME (*psym));
	    printf_filtered ("' only found in ");
	    puts_filtered (ps->filename);
	    printf_filtered (" psymtab\n");
	  }
	psym++;
      }
    b = BLOCKVECTOR_BLOCK (bv, GLOBAL_BLOCK);
    psym = ps->objfile->global_psymbols.list + ps->globals_offset;
    length = ps->n_global_syms;
    while (length--)
      {
	sym = lookup_block_symbol (b, SYMBOL_LINKAGE_NAME (*psym),
				   SYMBOL_DOMAIN (*psym));
	if (!sym)
	  {
	    printf_filtered ("Global symbol `");
	    puts_filtered (SYMBOL_LINKAGE_NAME (*psym));
	    printf_filtered ("' only found in ");
	    puts_filtered (ps->filename);
	    printf_filtered (" psymtab\n");
	  }
	psym++;
      }
    if (ps->texthigh < ps->textlow)
      {
	printf_filtered ("Psymtab ");
	puts_filtered (ps->filename);
	printf_filtered (" covers bad range ");
	fputs_filtered (paddress (gdbarch, ps->textlow), gdb_stdout);
	printf_filtered (" - ");
	fputs_filtered (paddress (gdbarch, ps->texthigh), gdb_stdout);
	printf_filtered ("\n");
	continue;
      }
    if (ps->texthigh == 0)
      continue;
    if (ps->textlow < BLOCK_START (b) || ps->texthigh > BLOCK_END (b))
      {
	printf_filtered ("Psymtab ");
	puts_filtered (ps->filename);
	printf_filtered (" covers ");
	fputs_filtered (paddress (gdbarch, ps->textlow), gdb_stdout);
	printf_filtered (" - ");
	fputs_filtered (paddress (gdbarch, ps->texthigh), gdb_stdout);
	printf_filtered (" but symtab covers only ");
	fputs_filtered (paddress (gdbarch, BLOCK_START (b)), gdb_stdout);
	printf_filtered (" - ");
	fputs_filtered (paddress (gdbarch, BLOCK_END (b)), gdb_stdout);
	printf_filtered ("\n");
      }
  }
}



void
map_partial_symbol_names (void (*fun) (const char *, void *), void *data)
{
  struct objfile *objfile;

  ALL_OBJFILES (objfile)
  {
    if (objfile->sf)
      objfile->sf->qf->map_symbol_names (objfile, fun, data);
  }
}

void
map_partial_symbol_filenames (void (*fun) (const char *, const char *,
					   void *),
			      void *data)
{
  struct objfile *objfile;

  ALL_OBJFILES (objfile)
  {
    if (objfile->sf)
      objfile->sf->qf->map_symbol_filenames (objfile, fun, data);
  }
}
