#ifndef VFKTABLEMODEL_H
#define VFKTABLEMODEL_H

#include <QSqlQueryModel>
#include <QStringList>



class VfkTableModel : public QSqlQueryModel
{
  Q_OBJECT
public:
  enum Nemovitost { NParcela, NBudova, NJednotka };
  enum OpravnenyPovinny { OPParcela, OPBudova, OPJednotka, OPOsoba };
  enum Pravo { Opravneni, Povinnost };

  explicit VfkTableModel(QString connectionName, QObject *parent = 0);
  QString value( int row, QString column ) const;
  bool telesa();
  bool telesoHlavicka( QString id );
  bool telesoParcely( QString cisloTel, bool extended );
  bool vlastnikParcely( QString opsubId, bool extended );
  bool telesoBudovy( QString cisloTel, bool extended );
  bool vlastnikBudovy( QString opsubId, bool extended );
  bool telesoJednotky( QString cisloTel, bool extended );
  bool vlastnikJednotky( QString opsubId, bool extended );
  bool parcela( QString id, bool extended );
  bool budova( QString id, bool extended );
  bool jednotka( QString id, bool extended );
  bool budovaJednotky( QString id );
  bool sousedniParcely( QString id );
  bool opravnenySubjekt ( QString id, bool extended );
  bool nemovitostTeleso( QString id, Nemovitost nemovitost );
  bool telesoVlastnici( QString id );
  bool nemovitostOchrana( QString id, Nemovitost nemovitost );
  bool vlastnikNemovitosti( QString id );
  bool parcelaBpej( QString id );
  bool nemovitostJpv( QString id, OpravnenyPovinny nemovitost, Pravo pravo, QString where = "");
  bool jpvListiny( QString id );
  bool nabyvaciListiny( QStringList parIds, QStringList budIds, QStringList jedIds );
  bool vlastnik( QString id, bool extended = true);
  bool dveRadyCislovani();
  bool definicniBod( QString id, Nemovitost nemovitost );

  bool searchOpsub( QString jmeno, QString identifikator, bool sjm, bool opo, bool ofo, QString lv );
  bool searchPar( QString parcelniCislo, QString typIndex, QString druhKod, QString lv );
  bool searchBud( QString domovniCislo, QString naParcele, QString zpusobVyuzitiKod, QString lv );
  bool searchJed( QString cisloJednotky, QString domovniCislo,
                  QString naParcele, QString zpusobVyuzitiKod, QString lv );
  QStringList parColumns( bool extended );
  QStringList budColumns( bool extended );
  QStringList jedColumns( bool extended );
  QStringList opsubColumns( bool extended = true);
  QStringList jpvColumns( bool extended );
  QStringList listinyColumns();
  QStringList bpejColumns();
  static QString nemovitost2TableName( Nemovitost nemovitost );
  static QString opravnenyPovinny2TableName( OpravnenyPovinny opravnenyPovinny );
  static QString pravo2ColumnSuffix( Pravo pravo );
  static OpravnenyPovinny tableName2OpravnenyPovinny( QString name );

  bool druhyPozemku( bool pozemkova = true, bool stavebni = true );
  bool zpusobVyuzitiBudov();
  bool zpusobVyuzitiJednotek();

signals:

public slots:

private:
  QString mConnectionName;

  bool evaluate( QString query );

};

#endif // VFKTABLEMODEL_H
