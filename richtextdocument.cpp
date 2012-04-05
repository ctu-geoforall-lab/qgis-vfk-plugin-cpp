#include "richtextdocument.h"

#include <QRegExp>

RichTextDocument::RichTextDocument()
{
}
void RichTextDocument::header()
{
  mPage += "<html><head></head><body>";
}

void RichTextDocument::footer()
{
  mPage += "</body></html>";
}

void RichTextDocument::heading1( const QString &text )
{
  mPage += QString( "<h1>%1</h1>" ).arg( text );
}

void RichTextDocument::heading2( const QString &text )
{
   mPage += QString( "<h2>%1</h2>" ).arg( text );
}

void RichTextDocument::heading3( const QString &text )
{
  mPage += QString( "<h3>%1</h3>" ).arg( text );
}

void RichTextDocument::beginItemize()
{
  mPage += "<ul>";
}

void RichTextDocument::endItemize()
{
  mPage += "</ul>";
}

void RichTextDocument::beginItem()
{
  mPage += "<li>";
}

void RichTextDocument::endItem()
{
  mPage += "</li>";
}

void RichTextDocument::item( const QString &text )
{
  mPage += QString( "<li>%1</li>" ).arg( text );
}

void RichTextDocument::beginTable()
{
  mPage += "<table>";
}

void RichTextDocument::endTable()
{
  mPage += "</table>";
}

void RichTextDocument::tableHeader( const QStringList &columns )
{
  mPage += "<tr>";
  foreach( QString column, columns )
  {
    mPage += QString( "<th>%1</th>" ).arg( column );
  }

  mPage += "</tr>";
  mLastColumnNumber = columns.size();
}

void RichTextDocument::tableRow( const QStringList &columns )
{
  mPage += "<tr>";
  foreach( QString column, columns )
  {
    mPage += QString( "<td>%1</td>" ).arg( column );
  }
  mPage += "</tr>";

  mLastColumnNumber = columns.size();
}

void RichTextDocument::tableRowOneColumnSpan(const QString &text)
{
  mPage += "<tr>";
  mPage += QString( "<td colspan=\"%1\">%2</td>" ).arg( mLastColumnNumber ).arg( text );
  mPage += "</tr>";
}

QString RichTextDocument::link( const QString &href, const QString &text )
{
  return QString( "<a href=\"%1\">%2</a>" ).arg( href ).arg( text );
}

QString RichTextDocument::superscript(const QString &text)
{
  return QString( "<sup>%1</sup>" ).arg( text );
}

QString RichTextDocument::newLine()
{
  return QString( "<br/>" );
}

void RichTextDocument::keyValueTable( const KeyValList &content )
{
  mPage += "<table>";

  for ( KeyValList::ConstIterator it = content.begin(); it != content.end(); ++it )
  {
    mPage += QString( "<tr>" );
    mPage += QString( "<td>%2</td>" ).arg( it->first );
    mPage += QString( "<td>%2</td>" ).arg( it->second );
    mPage += QString( "</tr>" );
  }
  mPage += QString( "</table>" );
}

void RichTextDocument::paragraph( const QString &text )
{
  mPage += QString( "<p>%1</p>" ).arg( text );
}

void RichTextDocument::table( const TableContent &content, bool header )
{
  beginTable();
  int i = 0;
  if ( header && !content.isEmpty() )
  {
    tableHeader( content.at( 0 ) );
    i++;
  }
  for ( ; i < content.size(); i++ )
  {
    tableRow( content.at( i ) );
  }

  endTable();
}

void RichTextDocument::text( const QString &text )
{
  mPage += text;
}

void RichTextDocument::discardLastBeginTable()
{
  int index = mPage.lastIndexOf( "<table" );
  mPage.chop( mPage.size() - index );
}

bool RichTextDocument::isLastTableEmpty()
{
  return mPage.contains( QRegExp( "<table[^>]*>$" ) );
}

