#include <locale>
#include "basecode/findMatch.h"
//#include "basecode/string/stringFunctions.h"

char* CC_strupr(char* str)
{
    char* ptr = str;
    while(*ptr != '\0')
    {
        if (islower(*ptr) )
        {
            *ptr = toupper(*ptr);
        }
            
            ++ptr;
    }
            
    return str;
}

FindMatch::FindMatch( uint32 _maxMatches )
{
   expression = NULL;
   maxMatches = _maxMatches;
   matchList.reserve( maxMatches );
}

FindMatch::FindMatch( char *_expression, uint32 _maxMatches )
{
   expression = NULL;
   setExpression( _expression );
   maxMatches = _maxMatches;
   matchList.reserve( maxMatches );
}

FindMatch::~FindMatch()
{
   delete [] expression;
   matchList.clear();
}

void FindMatch::setExpression( const char *_expression )
{
   delete [] expression;

   expression = new char[strlen(_expression) + 1];
   strcpy(expression, _expression);
   CC_strupr(expression);
}

bool FindMatch::findMatch( const char *str, bool caseSensitive )
{
   if ( isFull() )
      return false;

   char nstr[512];
   strcpy( nstr,str );
   CC_strupr(nstr);
   if ( isMatch( expression, nstr, caseSensitive ) )
   {
      matchList.push_back( (char*)str );
      return true;
   }
   return false;
}

inline bool IsCharMatch( char e, char s, bool caseSensitive )
{
   return ( ( e == '?' ) || ( caseSensitive && e == s ) || ( toupper(e) == toupper(s) ) );
}

bool FindMatch::isMatch( const char *exp, const char *str, bool caseSensitive )
{
   while ( *str && ( *exp != '*' ) )
   {
      if ( !IsCharMatch( *exp++, *str++, caseSensitive ) )
         return false;
   }

   const char* cp = NULL;
   const char* mp = NULL;

   while ( *str )
   {
      if ( *exp == '*' )
      {
         if ( !*++exp )
            return true;

         mp = exp;
         cp = str+1;
      }
      else if ( IsCharMatch( *exp, *str, caseSensitive ) )
      {
         exp++;
         str++;
      }
      else
      {
         exp = mp;
         str = cp++;
      }
   }

   while ( *exp == '*' )
      exp++;

   return !*exp;
}


bool FindMatch::isMatchMultipleExprs( const char *exps, const char *str, bool caseSensitive )
{
   char *tok = 0;
   int len = (int)strlen(exps);

   char *e = new char[len+1];
   strcpy(e,exps);

   for( tok = strtok(e,"\t"); tok != NULL; tok = strtok(NULL,"\t"))
   {
      if( isMatch( tok, str, caseSensitive) )
      {
         delete []e;
         return true;
      }
   }

   delete []e;
   return false;
}
