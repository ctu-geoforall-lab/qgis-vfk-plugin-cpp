#ifndef HTMLDOCUMENT_H
#define HTMLDOCUMENT_H

#include "vfkdocument.h"

class HtmlDocument: public VfkDocument
{
public:
  HtmlDocument();

  QString toString();
  void header();
  void footer();
  void heading1( const QString &text );
  void heading2( const QString &text );
  void heading3( const QString &text );
  void beginItemize();
  void endItemize();
  void beginItem();
  void endItem();
  void item( const QString &text );
  void beginTable();
  void endTable();
  void tableHeader( const QStringList &columns );
  void tableRow( const QStringList &columns );
  void tableRowOneColumnSpan( const QString &text );
  QString link( const QString &href, const QString &text );
  QString superscript( const QString &text );
  QString newLine();
  void keyValueTable( const KeyValList &content );
  void paragraph( const QString &text );
  void table( const TableContent &content, bool header );
  void text( const QString &text );
  void discardLastBeginTable();
  bool isLastTableEmpty();

  void title( const QString &text );

private:
 QString mPage;
 int mLastColumnNumber;
 bool titleIsSet;

};

#endif // HTMLDOCUMENT_H
