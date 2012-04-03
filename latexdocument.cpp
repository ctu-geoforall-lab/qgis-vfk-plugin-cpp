#include "latexdocument.h"

#include <QDebug>

LatexDocument::LatexDocument():
  mLastColumnNumber( 0 ),
  mMaxRows( 20 )
{
}

void LatexDocument::header()
{
  mPage += QString( "\\documentclass[a4paper,10pt]{article}\n"
                    "\\usepackage[utf8]{inputenc}\n"
                    "\\usepackage[czech]{babel}\n"
                    "\\usepackage{a4wide}\n"
                    "\\usepackage{tabulary}\n"
                    "\\begin{document}\n" );
}

void LatexDocument::footer()
{
  mPage += QString( "\\end{document}\n" );
}

void LatexDocument::heading1( const QString &text )
{
  mPage += QString( "\\section*{%1}\n" ).arg( text );
}

void LatexDocument::heading2( const QString &text )
{
  mPage += QString( "\\subsection*{%1}\n" ).arg( text );
}

void LatexDocument::heading3( const QString &text )
{
  mPage += QString( "\\subsubsection*{%1}\n" ).arg( text );
}

void LatexDocument::beginItemize()
{
  mPage += QString( "\\begin{itemize}\n" );
}

void LatexDocument::endItemize()
{
  mPage += QString( "\\end{itemize}\n" );
}

void LatexDocument::beginItem()
{
  mPage += QString( "\\item " );
}

void LatexDocument::endItem()
{
  mPage += QString( "\n" );
}

void LatexDocument::item( const QString &text )
{
  mPage += QString( "\\item %1\n" ).arg( text );
}

void LatexDocument::beginTable()
{
  mLastTableHeader.clear();
  mLastTableContent.clear();
}

void LatexDocument::endTable()
{
  QString table;
  if ( mLastColumnNumber > 0 )
  {
    bool tooLongTable = mLastTableContent.size() < mMaxRows ? false : true;
    QString beginTable = "\\begin{tabulary}{\\textwidth}{";
    QString endTable = "\\end{tabulary}\n";
    QString header = mLastTableHeader + "\\\\\\hline\\hline\n";
    for ( int i = 0; i < mLastColumnNumber; i++ )
    {
     beginTable += "L";
    }
    beginTable += "}\n";


    if ( tooLongTable )
    {
      int rows = 0;
      while ( rows + mMaxRows < mLastTableContent.size() )
      {
        table += beginTable;
        table += header;
        for ( int i = rows; i < rows + mMaxRows; i++ )
        {
          table += mLastTableContent.at( i );
          table += "\\\\\n";
        }
        table += endTable;
        table += "\\newpage";
        rows += mMaxRows;
      }
      table += beginTable;
      table += header;
      for ( int i = rows; i < mLastTableContent.size(); i++ )
      {
        table += mLastTableContent.at( i );
        table += "\\\\\n";
      }
      table += endTable;
    }
    else
    {
      table += beginTable;
      table += header;
      table += mLastTableContent.join( "\\\\\n" );
      table += endTable;
    }
  }
  mPage += table;
}

void LatexDocument::tableHeader( const QStringList &columns )
{
  mLastColumnNumber = columns.size();
  if ( mLastColumnNumber > 0 )
  {
    QString tableHeader;
    for ( QStringList::ConstIterator it = columns.begin(); it != columns.end(); ++it )
    {
      QString columnHeader = QString( "\\textbf{%1} & " ).arg( *it );
      QStringList words = ( *it ).split( " " );
      if ( words.size() == 1 )
      {
        columnHeader = QString( "\\mbox{\\textbf{%1}} & " ).arg( *it );
      }
      tableHeader += columnHeader;
    }
    tableHeader.chop( 2 );
    mLastTableHeader = tableHeader;
  }

}

void LatexDocument::tableRow( const QStringList &columns )
{
  if ( mLastColumnNumber != columns.size() )
  {
    qWarning() << "inconsistent number of columns: " << mLastColumnNumber << columns.size();
    return;
  }
  if ( mLastColumnNumber > 0 )
  {
    QString tableRow = QString( "%1 " ).arg( columns.at( 0 ) );

    for ( QStringList::ConstIterator it = ++( columns.begin() ); it != columns.end(); ++it )
    {
      tableRow += QString( "& %1 " ).arg( *it );
    }
    mLastTableContent.append( tableRow );
  }
}

void LatexDocument::tableRowOneColumnSpan( const QString &text )
{
   if ( mLastColumnNumber != 0 )
   {
     mLastTableContent.append( QString( "\\multicolumn{%1}{l}{%2}" ).arg( mLastColumnNumber ).arg( text ) );
   }
}

QString LatexDocument::link( const QString &href, const QString &text )
{
  return text;
}

QString LatexDocument::superscript( const QString &text )
{
  return QString( "$^{%1}$" ).arg( text );
}

QString LatexDocument::newLine()
{
  return QString( "\\newline\n" );
}

void LatexDocument::keyValueTable( const KeyValList &content )
{
  mPage += QString( "\\begin{tabulary}{\\textwidth}{LL}\n" );
  for ( KeyValList::ConstIterator it = content.begin(); it != content.end(); ++it )
  {
    mPage += QString( "\\textbf{%1} & %2 \\\\\n" ).arg( it->first ).arg( it->second );
  }
  mPage += QString( "\\end{tabulary}\n" );
}

void LatexDocument::paragraph( const QString &text )
{
  mPage += QString( "\n%1\n" ).arg( text );
}

void LatexDocument::table( const TableContent &content, bool header )
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

void LatexDocument::text( const QString &text )
{
  mPage += QString( "%1\n" ).arg( text );
}

void LatexDocument::discardLastBeginTable()
{
}

bool LatexDocument::isLastTableEmpty()
{
  return false;
}











