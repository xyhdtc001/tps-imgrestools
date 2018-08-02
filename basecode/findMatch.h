
#ifndef _FINDMATCH_H_
#define _FINDMATCH_H_

#include <vector>
#include "basecode/basehdr.h"

class   FindMatch
{
   char*  expression;
   uint32 maxMatches;

  public:
   static bool isMatch( const char *exp, const char *string, bool caseSensitive = false );
   static bool isMatchMultipleExprs( const char *exps, const char *str, bool caseSensitive );
   std::vector<char *> matchList;

   FindMatch( uint32 _maxMatches = 256 );
   FindMatch( char *_expression, uint32 _maxMatches = 256 );
   ~FindMatch();

   bool findMatch(const char *string, bool caseSensitive = false);
   void setExpression( const char *_expression );

   uint32  numMatches() const   
   {
      return (uint32)matchList.size();
   }

   bool isFull() const       
   {
      return (matchList.size() >= int32(maxMatches)); 
   }

   void clear()              
   {
      matchList.clear();                       
   }
};

#endif // _FINDMATCH_H_
