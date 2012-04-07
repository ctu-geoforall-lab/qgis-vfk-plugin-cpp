#ifndef HTMLBUILDER_H
#define HTMLBUILDER_H

#include <QUrl>
#include <QSqlQueryModel>

#include "vfktablemodel.h"
#include "htmldocument.h"

typedef QMap<QString, QString > TaskMap;
typedef QMap<QString, QString > StringMap;
typedef QPair<QString, QString> Coordinates;

class DocumentBuilder
{
public:
  DocumentBuilder( QString connectionName );
  ~DocumentBuilder();
  bool buildHtml( VfkDocument *document, TaskMap taskMap );
  void initKatUzemi();
  QStringList currentParIds() { return mCurrentPageParIds; }
  QStringList currentBudIds() { return mCurrentPageBudIds; }
  Coordinates currentDefinitionPoint() { return mCurrentDefinitionPoint; }


private:
  QString mConnectionName;
  bool mDveRadyCislovani;
  VfkDocument *mDocument;
  QStringList mCurrentPageParIds;
  QStringList mCurrentPageBudIds;
  Coordinates mCurrentDefinitionPoint;

  QString mStringBezZapisu;

  void pageTelesa();
  void pageTeleso( QString id );
  void partTelesoHlavicka( QString id );
  void partTelesoNemovitosti( QString id, QStringList &parIds,
                                 QStringList &budIds, QStringList &jedIds );
  void partVlastnikNemovitosti( QString opsubId );
  void partTelesoParcely( QString opsubId, QStringList &parIds );
  void partVlastnikParcely( QString id );
  void tableParcely( const VfkTableModel *model, QStringList &parIds, bool LVcolumn );
  void partTelesoBudovy( QString opsubId, QStringList &budIds );
  void partVlastnikBudovy( QString id );
  void tableBudovy( const VfkTableModel *model, QStringList &budIds, bool LVcolumn );
  void partTelesoJednotky( QString id, QStringList &jedIds );
  void partVlastnikJednotky( QString opsubId );
  void tableJednotky( const VfkTableModel *model, QStringList &jedIds, bool LVcolumn );
  void partTelesoJednotkaDetail( QString budId );
  void partTelesoB1( const QStringList &parIds,
                         const QStringList &budIds, const QStringList &jedIds,
                         const QStringList &opsubIds, bool forLV );
  void partTelesoC( const QStringList &parIds,
                       const QStringList &budIds, const QStringList &jedIds,
                       const QStringList &opsubIds, bool forLV );
  void partTelesoD( const QStringList &parIds,
                       const QStringList &budIds, const QStringList &jedIds,
                       const QStringList &opsubIds, bool forLV );
  void partTelesoE( const QStringList &parIds,
                       const QStringList &budIds, const QStringList &jedIds );
  void partTelesoF( const QStringList &parIds, bool forLV );
  bool partTelesoJinaPrava( const QStringList ids, VfkTableModel::OpravnenyPovinny opravneny );
  bool partNemovitostJinaPrava(const QStringList ids, VfkTableModel::OpravnenyPovinny opravneny);
  bool partTelesoOmezeniPrava( const QStringList ids, VfkTableModel::OpravnenyPovinny povinny );
  bool partNemovitostOmezeniPrava( const QStringList ids, VfkTableModel::OpravnenyPovinny povinny );
  bool partTelesoJineZapisy( const QStringList ids, VfkTableModel::OpravnenyPovinny povinny );
  bool partNemovitostJineZapisy( const QStringList ids, VfkTableModel::OpravnenyPovinny povinny );
  bool partTelesoB1CDSubjekt( const QStringList ids,
                                 VfkTableModel::OpravnenyPovinny pravniSubjekt,
                                 VfkTableModel::Pravo pravo, bool sekceD, bool showListiny );
  void partTelesoListiny( QString jpvId );
  void pageParcela( QString id );
  void partTelesoVlastnici( QString id, QStringList &opsubIds, bool forLV );
//  QString partParcelaBpej( QString id );
  void partNemovitostOchrana( QString id, VfkTableModel::Nemovitost nemovitost);
//  QString partParcelaJpv( QString id, VfkTableModel::Pravo pravo);
  void pageBudova( QString id );
  void pageJednotka( QString id );
  void pageOpravnenySubjekt( QString id );


  void pageSeznamParcel( QStringList ids );
  void pageSeznamOsob( QStringList ids );
  void pageSeznamBudov(  QStringList ids );
  void pageSeznamJednotek( QStringList ids );
//  void pageSeznamOsob( QString name );

  void pageSearchVlastnici( QString jmeno, QString identifikator, bool sjm, bool opo, bool ofo, QString lv );
  void pageSearchParcely( QString parcelniCislo, QString typIndex, QString druhKod, QString lv );
  void pageSearchBudovy( QString domovniCislo, QString naParcele, QString zpusobVyuziti, QString lv );
  void pageSearchJednotky( QString cisloJednotky, QString domovniCislo,
                           QString naParcele, QString zpusobVyuziti, QString lv );
  QString makeShortDescription( QString id, VfkTableModel::OpravnenyPovinny nemovitost );
  QString makeLongDescription( QString id, VfkTableModel::OpravnenyPovinny nemovitost );
  QString makeAdresa( const VfkTableModel *model, int row );
  QString makeJmeno( const VfkTableModel *model, int row );
  QString makeIdentifikator( const VfkTableModel *model, int row );
  QString makeParcelniCislo( const VfkTableModel *model, int row );
  QString makeDomovniCislo( const VfkTableModel *model, int row );
  QString makeJednotka( const VfkTableModel *model, int row );
  QString makeListina( const VfkTableModel *model, int row );
  QString makeLVCislo( const VfkTableModel *model, int row );
  QString makeKatastrUzemi( const VfkTableModel *model, int row );
  QString makeCastObce(const VfkTableModel *model, int row);
  QString makeObec(const VfkTableModel *model, int row);

  void saveDefinitionPoint( QString id, VfkTableModel::Nemovitost nemovitost);
};

#endif // HTMLBUILDER_H
