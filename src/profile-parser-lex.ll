%{
#include <cstdlib>
#include <errno.h>
#include <limits.h>
#include <string>
#include "profile-parser-driver.hpp"
#include "profile-parser-grammar.hpp"

	/* Work around an incompatibility in flex (at least versions
	   2.5.31 through 2.5.33): it generates code that does
	   not conform to C89.  See Debian bug 333231
	   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
#undef yywrap
#define yywrap() 1

	/* By default yylex returns int, we use token_type.
	   Unfortunately yyterminate by default returns 0, which is
	   not of token_type.  */
#define yyterminate() return token::END
%}

%option noyywrap nounput batch debug

/* abreviations */
id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t]

%{
# define YY_USER_ACTION  yylloc->columns (yyleng);
%}

%%
%{
yylloc->step ();
%}

{blank}+   yylloc->step ();
[\n]+      yylloc->lines (yyleng); yylloc->step;


%%
void ProfileParserDriver::scan_begin ()
{
	yy_flex_debug = trace_scanning;
	if (file == "-")
		yyin = stdin;
	else if (!(yyin = fopen (file.c_str (), "r")))
	{
		error (std::string ("cannot open ") + file);
		exit (1);
	}
}

void ProfileParserDriver::scan_end ()
{
	fclose (yyin);
}


/*
###################
# Error and Lexer
###################

sub yyerror {
	my ($msg, $fh) = @_;
	die "$msg at line $. .\n";
}

my @line_fields = ();
sub yylex {
	my ($fh) = shift;
	while($#line_fields eq -1) {
		# load a new line
		$_ = $fh->getline;
		if( ! defined $_ ) {
			return 0;
		}
		
		# strip comments
		s/#.*$//;

		# special caracters
		s/\"/ \" /;
		s/{/ { /;
		s/}/ } /;
		s/=/ = /;

		chomp;
		s/^\s+//;
		@line_fields = split(/\s+/);
	}
	my $word = shift(@line_fields);

	# now we check which token 
	if ($word eq "mount") {
		return ($TOKMOUNT, $word);
	}
	elsif($word eq "mode") {
		return ($TOKMODE, $word);
	}
	elsif($word eq "name") {
		return ($TOKNAME, $word);
	}
	elsif($word eq "cpu") {
		return ($TOKCPU, $word);
	}
	elsif($word eq "profile" ) {
		return ($TOKPROFILE, $word);
	}
	elsif($word eq '=' ) {
		return ($EQUAL, $word);
	}
	elsif($word eq '{' ) {
		return ($OBRACK, $word);
	}
	elsif($word eq '}' ) {
		return ($CBRACK,$word);
	}
	elsif($word =~ /[0-5]?\d:[0-5]?\d:[0-5]?\d/ ) {
		return ($TIME, $word );
	}
	elsif($word =~ /\d+/ ) {
		return ($NUMBER, $word);
	}		
	elsif($word =~ /\w+/ ) {
		return ($WORD, $word );
	}
	elsif($word =~ /[\w\d\/\.]+/) {
		return ($PATH, $word );
	}
	else {
		return ord($word);
	}
}

sub custom_new {
	my $p = bless { yydata => () }, $_[0];
	$p->{yylex} = $_[1];
	$p->{yyerror} = $_[2];
	$p->{yydebug} = $_[3];
	return $p;
}
*/
