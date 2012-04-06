#ifndef RICHTEXTDOCUMENT_H
#define RICHTEXTDOCUMENT_H

#include "vfkdocument.h"

class RichTextDocument : public VfkDocument
{
public:
  RichTextDocument();

  QString toString() { return mPage; }
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

private:
  QString currentTableRowCssClass();

  QString mPage;
  int mLastColumnNumber;
  int mCurrentTableRowNumber; ///< starts from 1 (not from 0) because of recognition of odd and even rows
};

#endif // RICHTEXTDOCUMENT_H
