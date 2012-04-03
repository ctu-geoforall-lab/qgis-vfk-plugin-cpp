#ifndef VFKDOCUMENT_H
#define VFKDOCUMENT_H

#include <QString>
#include <QStringList>
#include <QPair>

typedef QList<QPair<QString, QString> > KeyValList;
typedef QList<QStringList> TableContent;

class VfkDocument
{
public:
  virtual ~VfkDocument() = 0;

  virtual QString toString() = 0;
  virtual void header() = 0;
  virtual void footer() = 0;
  virtual void heading1( const QString &text ) = 0;
  virtual void heading2( const QString &text ) = 0;
  virtual void heading3( const QString &text ) = 0;
  virtual void beginItemize() = 0;
  virtual void endItemize() = 0;
  virtual void beginItem() = 0;
  virtual void endItem() = 0;
  virtual void item( const QString &text ) = 0;
  virtual void beginTable() = 0;
  virtual void endTable() = 0;
  virtual void tableHeader( const QStringList &columns ) = 0;
  virtual void tableRow( const QStringList &columns ) = 0;
  virtual void tableRowOneColumnSpan( const QString &text ) = 0;
  virtual QString link( const QString &href, const QString &text ) = 0;
  virtual QString superscript( const QString &text ) = 0;
  virtual QString newLine() = 0;
  virtual void keyValueTable( const KeyValList &content ) = 0;
  virtual void paragraph( const QString &text ) = 0;
  virtual void table( const TableContent &content, bool header ) = 0;
  virtual void text( const QString &text ) = 0;
  virtual void discardLastBeginTable() = 0;
  virtual bool isLastTableEmpty() = 0;

};

inline VfkDocument::~VfkDocument() {}

#endif // VFKDOCUMENT_H
