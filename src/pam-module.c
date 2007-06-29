
#include "pam-module.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

int
def_parse_config( pam_module_t *this __attribute__ ((unused)), char *arguments, write_type_t type __attribute__ ((unused))){
  if (arguments){
    return 0;
  }
  return 1;
}

static void
def_print_module_type (pam_module_t *this, write_type_t type)
{
  option_set_t *opt_set = this->get_opt_set (this, type);

  if (!opt_set->is_enabled (opt_set, "is_enabled"))
    return;

  printf ("%s:", type2string( type ) );
  for_each_bool_opt( opt_set,  &print_bool_opt );
  for_each_string_opt( opt_set, &print_string_opt );
  printf( "\n" );
}

int
def_print_module (pam_module_t *this)
{
  def_print_module_type (this, AUTH);
  def_print_module_type (this, ACCOUNT);
  def_print_module_type (this, PASSWORD);
  def_print_module_type (this, SESSION);

  return TRUE;
}

int
def_write_config( pam_module_t *this, enum write_type op __attribute__ ((unused)), FILE *fp __attribute__ ((unused)) ){
  printf( "default write module:\t%s\n", this->name );
  return 1;
}

pam_module_t* lookup( pam_module_t **module_list, const char *module ){
  while (*module_list != NULL) {
    if( 0 == strcmp( (*module_list)->name, module ) ){
      return *module_list;
    }
    module_list++;
  };
  return *module_list;
}

int
is_module_enabled (pam_module_t **module_list, const char *module,
		   write_type_t op)
{
  pam_module_t *mod = lookup (module_list, module);

  if (mod == NULL)
    return FALSE;

  option_set_t *opt_set = mod->get_opt_set (mod,op);

  return opt_set->is_enabled (opt_set, "is_enabled");
}

const char *
type2string (write_type_t wt)
{
  if( ACCOUNT == wt ) return "account";
  else if( AUTH == wt ) return "auth";
  else if( PASSWORD == wt ) return "password";
  else if( SESSION == wt ) return "session";
  else return "<unknown type>";
}

int
handle_module( const char *file, char *m, char *arguments , pam_module_t **module_list, write_type_t type ){
  pam_module_t *module = lookup( module_list, m );
  if( NULL != module ){
    if( ! module->parse_config( module, arguments, type ) ){
      fprintf (stderr, _("%s (%s): Arguments will be ignored\n"),  file, m );
    }
  }
  else{
    fprintf (stderr, _("%s: Unknown module %s, ignored!\n"), file, m);
  }
  return 1;
}

void
print_module_config (pam_module_t **module_list, const char *module)
{
  while (*module_list != NULL)
    {
      if (strcmp ((*module_list)->name, module) == 0)
        (*module_list)->print_module( (*module_list) );
      module_list++;
    }
}

void
print_unknown_option_error (const char *module, const char *option)
{
  /* TRANSLATORS: first argument is name of a PAM module */
  fprintf (stderr, _("Unknown option for %s, ignored: '%s'\n"),
	   module, option);
}

option_set_t*
get_opt_set( pam_module_t *this, write_type_t op ){
  return (this->option_sets)[op];
}
