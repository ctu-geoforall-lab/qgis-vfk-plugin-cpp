#include "domains.h"

Domains::Domains()
{
}

bool Domains::anoNe( QString an )
{
  if ( an == "a" )
  {
    return true;
  }
  else
  {
    return false;
  }
}

QString Domains::cpCe( int kod )
{
  switch( kod )
  {
  case 1:
    return "Číslo popisné";
  case 2:
    return "Číslo evidenční";
  default:
    return "";
  }
}

QString Domains::druhUcastnika(int kod)
{
  switch( kod )
  {
  case 1:
    return "právnická osoba";
  case 2:
    return "fyzická osoba";
  case 3:
    return "ostatní";
  case 4:
    return "právnická osoba státní správy";
  default:
    return "";
  }
}

QString Domains::rodinnyStav(int kod)
{
  switch( kod )
  {
  case 1:
    return "svobodný/svobodná";
  case 2:
    return "ženatý/vdaná";
  case 3:
    return "rozvedený/rozvedená";
  case 4:
    return "ovdovělý/ovdovělá";
  case 5:
    return "druh/družka";
  default:
    return "";
  }
}
