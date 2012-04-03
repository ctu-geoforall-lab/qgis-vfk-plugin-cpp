#ifndef DOMAINS_H
#define DOMAINS_H

#include <QString>

class Domains
{
public:
  Domains();
  static bool anoNe( QString an );
  static QString cpCe( int kod );
  static QString druhUcastnika( int kod );
  static QString rodinnyStav( int kod );
};

#endif // DOMAINS_H
