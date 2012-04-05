#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>
#include <QTime>
#include "vfktablemodel.h"

VfkTableModel::VfkTableModel(  QString connectionName, QObject *parent ) :
  QSqlQueryModel( parent ),
  mConnectionName( connectionName )
{
}

bool VfkTableModel::telesa()
{
  QString query = QString( "SELECT tel.id tel_id, "
                           "tel.katuze_kod tel_katuze_kod, "
                           "tel.cislo_tel tel_cislo_tel "
                           "FROM tel;" );

  return evaluate( query );
}

bool VfkTableModel::telesoHlavicka( QString id )
{
  QString query = QString( "SELECT tel.id tel_id, tel.cislo_tel tel_cislo_tel, "
                           "katuze.kod katuze_kod, katuze.nazev katuze_nazev, "
                           "obce.kod obce_kod, obce.nazev obce_nazev, "
                           "okresy.nuts4 okresy_nuts4, okresy.nazev okresy_nazev "
                           "FROM tel "
                           "JOIN katuze ON tel.katuze_kod = katuze.kod "
                           "JOIN obce ON katuze.obce_kod = obce.kod "
                           "JOIN okresy ON obce.okresy_kod = okresy.kod "
                           "WHERE tel.id = %1;" ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::telesoParcely( QString cisloTel, bool extended )
{
  QString columns = parColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM tel "
                           "JOIN par ON par.tel_id = tel.id "
                           "LEFT JOIN drupoz ON par.drupoz_kod = drupoz.kod "
                           "LEFT JOIN zpvypo ON par.zpvypa_kod = zpvypo.kod "
                           "WHERE tel.id = %2;" ).arg( columns ).arg( cisloTel );


  return evaluate( query );
}

bool VfkTableModel::vlastnikParcely( QString opsubId, bool extended )
{
  // index on vla.opsub_id!
  QString columns = parColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM tel "
                           "JOIN vla ON vla.tel_id = tel.id "
                           "JOIN opsub ON vla.opsub_id = opsub.id "
                           "JOIN par ON par.tel_id = tel.id "
                           "LEFT JOIN drupoz ON par.drupoz_kod = drupoz.kod "
                           "LEFT JOIN zpvypo ON par.zpvypa_kod = zpvypo.kod "
                           "WHERE opsub.id = %2;" ).arg( columns ).arg( opsubId );


  return evaluate( query );
}

bool VfkTableModel::telesoBudovy( QString cisloTel, bool extended )
{
  QString columns = budColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM tel "
                           "JOIN bud ON bud.tel_id = tel.id "
                           "JOIN typbud ON typbud.kod = bud.typbud_kod "
                           "JOIN par ON par.bud_id = bud.id "
                           "LEFT JOIN zpvybu ON zpvybu.kod = bud.zpvybu_kod "
                           "LEFT JOIN casobc ON casobc.kod = bud.caobce_kod "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "WHERE tel.id = %2;" ).arg( columns ).arg( cisloTel );

  return evaluate( query );
}

bool VfkTableModel::vlastnikBudovy( QString opsubId, bool extended )
{
  QString columns = budColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM tel "
                           "JOIN vla ON vla.tel_id = tel.id "
                           "JOIN opsub ON vla.opsub_id = opsub.id "
                           "JOIN bud ON bud.tel_id = tel.id "
                           "JOIN typbud ON typbud.kod = bud.typbud_kod "
                           "JOIN par ON par.bud_id = bud.id "
                           "LEFT JOIN zpvybu ON zpvybu.kod = bud.zpvybu_kod "
                           "LEFT JOIN casobc ON casobc.kod = bud.caobce_kod "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "WHERE opsub.id = %2;" ).arg( columns ).arg( opsubId );


  return evaluate( query );
}

bool VfkTableModel::telesoJednotky ( QString cisloTel, bool extended )
{
  QString columns = jedColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM jed "
                           "JOIN tel ON tel.id = jed.tel_id "
                           "JOIN bud ON jed.bud_id = bud.id "
                           "JOIN typjed ON typjed.kod = jed.typjed_kod "
                           "LEFT JOIN zpvyje ON zpvyje.kod = jed.zpvyje_kod "
                           "JOIN par ON par.bud_id = bud.id "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "WHERE tel.id = %2;" ).arg( columns ).arg( cisloTel );

  return evaluate( query );
}

bool VfkTableModel::vlastnikJednotky( QString opsubId, bool extended )
{
  QString columns = jedColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM jed "
                           "JOIN tel ON tel.id = jed.tel_id "
                           "JOIN vla ON vla.tel_id = tel.id "
                           "JOIN opsub ON vla.opsub_id = opsub.id "
                           "JOIN bud ON jed.bud_id = bud.id "
                           "JOIN typjed ON typjed.kod = jed.typjed_kod "
                           "LEFT JOIN zpvyje ON zpvyje.kod = jed.zpvyje_kod "
                           "JOIN par ON par.bud_id = bud.id "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "WHERE opsub.id = %2;" ).arg( columns ).arg( opsubId );

  return evaluate( query );
}

bool VfkTableModel::parcela( QString id, bool extended )
{
  QString columns = parColumns( extended ).join( ", " );
  QString query = QString("SELECT DISTINCT %1 "
                          "FROM par "
                          "LEFT JOIN tel ON par.tel_id = tel.id "
                          "LEFT JOIN zpurvy ON par.zpurvy_kod = zpurvy.kod "
                          "LEFT JOIN drupoz ON par.drupoz_kod = drupoz.kod "
                          "LEFT JOIN zpvypo ON par.zpvypa_kod = zpvypo.kod "
                          "LEFT JOIN maplis ON par.maplis_kod = maplis.id "
                          "JOIN katuze ON par.katuze_kod = katuze.kod "
                          "LEFT JOIN bud ON par.bud_id = bud.id "
                          "LEFT JOIN typbud ON bud.typbud_kod = typbud.kod "
                          "WHERE par.id = %2;" ).arg( columns ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::budova( QString id, bool extended )
{
  QString columns = budColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM bud "
                           "JOIN typbud ON typbud.kod = bud.typbud_kod "
                           "JOIN par ON par.bud_id = bud.id "
                           "LEFT JOIN tel ON bud.tel_id = tel.id "
                           "LEFT JOIN zpvybu ON zpvybu.kod = bud.zpvybu_kod "
                           "LEFT JOIN casobc ON casobc.kod = bud.caobce_kod "
                           "JOIN katuze ON par.katuze_kod = katuze.kod "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "WHERE bud.id = %2;" ).arg( columns ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::jednotka( QString id, bool extended )
{
  QString columns = jedColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM jed "
                           "JOIN typjed ON typjed.kod = jed.typjed_kod "
                           "LEFT JOIN zpvyje ON zpvyje.kod = jed.zpvyje_kod "
                           "LEFT JOIN bud ON bud.id = jed.bud_id "
                           "JOIN typbud ON typbud.kod = bud.typbud_kod "
                           "JOIN par on par.bud_id = bud.id "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "JOIN katuze ON par.katuze_kod = katuze.kod "
                           "JOIN tel ON tel.id = jed.tel_id "
                           "WHERE jed.id = %2;" ).arg( columns ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::budovaJednotky( QString id )
{
  QString query = QString( "SELECT jed.id jed_id, "
                           "jed.cislo_jednotky jed_cislo_jednotky, "
                           "bud.cislo_domovni bud_cislo_domovni "
                           "FROM bud "
                           "JOIN jed ON bud.id = jed.bud_id "
                           "WHERE bud.id = %1;" ).arg( id );
  return evaluate( query );
}

bool VfkTableModel::sousedniParcely( QString id )
{
  QString query = QString( "SELECT DISTINCT hp.par_id_1 hp_par_id_1, "
                           "hp.par_id_2 hp_par_id_2 "
                           "FROM hp "
                           "WHERE hp.par_id_1 = %1 "
                           "OR hp.par_id_2 = %1;" ).arg( id );
  return evaluate( query );
}

bool VfkTableModel::opravnenySubjekt( QString id, bool extended )
{
  QString columns = opsubColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM opsub "
                           "JOIN charos ON charos.kod = opsub.charos_kod "
                           "WHERE opsub.id = %2;" ).arg( columns ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::nemovitostTeleso( QString id, VfkTableModel::Nemovitost nemovitost )
{
  QString table = nemovitost2TableName( nemovitost );
  QString query = QString( "SELECT tel.id tel_id, tel.cislo_tel tel_cislo_tel "
                           "FROM tel "
                           "JOIN %1 ON %1.tel_id = tel.id "
                           "WHERE %1.id = %2;" ).arg( table ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::telesoVlastnici( QString id )
{
  QString query = QString( "SELECT "
                           "vla.id vla_id, "
                           "vla.opsub_id vla_opsub_id, "
                           "vla.podil_citatel vla_podil_citatel, "
                           "vla.podil_jmenovatel vla_podil_jmenovatel, "
                           "typrav.nazev typrav_nazev, "
                           "typrav.sekce typrav_sekce "
                           "FROM vla "
                           "JOIN tel ON vla.tel_id = tel.id "
                           "JOIN typrav ON typrav.kod = vla.typrav_kod "
                           "WHERE tel.id = %1 ORDER BY typrav.sekce;" ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::nemovitostOchrana( QString id, Nemovitost nemovitost )
{
  QString table = nemovitost2TableName( nemovitost );

  QString query = QString( "SELECT zpochn.nazev zpochn_nazev "
                           "FROM zpochn "
                           "JOIN %1 ON rzo.%1_id = %1.id "
                           "JOIN rzo ON rzo.zpochr_kod = zpochn.kod "
                           "WHERE %1.id = %2;" ).arg( table ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::vlastnikNemovitosti( QString id )
{
  QString query = QString( "SELECT tel.cislo_tel tel_cislo_tel, "
                           "tel.id tel_id, "
                           "par.id par_id, "
                           "bud.id bud_id, "
                           "jed.id jed_id "
                           "FROM tel "
                           "JOIN vla ON vla.tel_id = tel.id "
                           "JOIN opsub ON vla.opsub_id = opsub.id "
                           "LEFT JOIN par ON par.tel_id = tel.id "
                           "LEFT JOIN bud ON bud.tel_id = tel.id "
                           "LEFT JOIN jed ON jed.tel_id = tel.id "
                           "WHERE opsub.id = %1;" ).arg( id );
  return evaluate( query );
}

bool VfkTableModel::parcelaBpej( QString id )
{
  QString columns = bpejColumns().join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM bdp "
                           "JOIN par ON bdp.par_id = par.id "
                           "LEFT JOIN drupoz ON drupoz.kod = par.drupoz_kod "
                           "WHERE par.id = %2;" ).arg( columns ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::nemovitostJpv( QString id, OpravnenyPovinny op, Pravo pravo, QString where )
{
  QString table = opravnenyPovinny2TableName( op );

  QString columnNameSuffix = pravo2ColumnSuffix( pravo );

  QString columns = jpvColumns( false ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM jpv "
                           "JOIN %4 ON %4.id = jpv.%4_id_%2 "
                           "JOIN typrav ON typrav.kod = jpv.typrav_kod "
                           "WHERE %4.id = %3%5;" )
      .arg( columns )
      .arg( columnNameSuffix )
      .arg( id ).arg( table )
      .arg( where.isEmpty() ? "" : " AND " + where );

  return evaluate( query );
}

bool VfkTableModel::jpvListiny( QString id )
{
  QString columns = listinyColumns().join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM jpv "
                           "JOIN rl ON rl.jpv_id = jpv.id "
                           "JOIN listin ON rl.listin_id = listin.id "
                           "JOIN ldu ON ldu.listin_id=listin.id "
                           "join dul ON dul.kod = ldu.dul_kod "
                           "JOIN typlis ON typlis.kod=listin.typlist_kod "
                           "WHERE jpv.id = %2;" ).arg( columns ).arg( id );

  return evaluate( query );
}

bool VfkTableModel::nabyvaciListiny( QStringList parIds, QStringList budIds, QStringList jedIds )
{
  QString columns = listinyColumns().join( ", " );
  // the duration depends a lot on join order
  QString query = QString( "SELECT DISTINCT %1 "
                           "FROM rl "
                           "JOIN listin ON rl.listin_id = listin.id "
                           "LEFT JOIN par ON par.id=rl.par_id "
                           "LEFT JOIN bud ON bud.id=rl.bud_id "
                           "LEFT JOIN jed ON jed.id=rl.jed_id "
                           "JOIN ldu ON ldu.listin_id=listin.id "
                           "JOIN dul ON dul.kod = ldu.dul_kod "
                           "JOIN typlis ON typlis.kod=listin.typlist_kod "
                           "WHERE par.id in (%2) "
                           "OR bud.id in (%3) "
                           "OR jed.id in (%4) "
                           "ORDER BY rl.listin_id;" )
      .arg( columns )
      .arg( parIds.join( "," ) )
      .arg( budIds.join( "," ) )
      .arg( jedIds.join( "," ) );
  return evaluate( query );
}

bool VfkTableModel::vlastnik( QString id, bool extended )
{
  QString columns = opsubColumns( extended ).join( ", " );
  QString query = QString( "SELECT %1 "
                           "FROM opsub "
                           "JOIN charos ON opsub.charos_kod = charos.kod "
                           "WHERE opsub.id = %2;" ).arg( columns ).arg( id );
  return evaluate( query );
}

bool VfkTableModel::dveRadyCislovani()
{
  QString query = QString( "SELECT 1 from FROM where druh_cislovani_par = 1" );
  setQuery( query, QSqlDatabase::database( mConnectionName ) );
  if ( rowCount() > 0 )
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool VfkTableModel::searchOpsub( QString jmeno, QString identifikator, bool sjm, bool opo, bool ofo, QString lv )
{
  QString whereJmeno;
  QString join;
  if ( !jmeno.isEmpty() )
  {
    if ( ofo )
    {
      whereJmeno += QString( "opsub.jmeno LIKE \"\%%1\%\" OR opsub.prijmeni LIKE \"\%%1\%\" OR " ).arg( jmeno );
    }
    if ( sjm || opo )
    {
      whereJmeno += QString( "opsub.nazev LIKE \"\%%1\%\" OR " ).arg( jmeno );
    }
    whereJmeno += "0 "; // or 0
  }
  QString whereIdent;
  if ( !identifikator.isEmpty() )
  {
    if ( ofo )
    {
      whereIdent += QString( "opsub.rodne_cislo = %1 OR " ).arg( identifikator );
    }
    if ( opo )
    {
      whereIdent += QString( "opsub.ico = %1 OR " ).arg( identifikator );
    }
    whereIdent += "0 "; // or 0
  }

  QStringList opsubType;
  if ( ofo )
  {
    opsubType << "\"OFO\"";
  }
  if ( opo )
  {
    opsubType << "\"OPO\"";
  }
  if ( sjm )
  {
    opsubType << "\"BSM\"";
  }

  QString where = "WHERE ";

  if ( !whereJmeno.isEmpty() )
  {
    where += QString( "( %1 ) AND " ).arg( whereJmeno );
  }
  if ( !whereIdent.isEmpty() )
  {
    where += QString( "(%1) AND " ).arg( whereIdent );
  }

  if ( !lv.isEmpty() )
  {
    where += QString( "tel.cislo_tel = %1 AND " ).arg( lv );
    join += QString( "JOIN vla ON vla.opsub_id = opsub.id "
                     "JOIN tel ON vla.tel_id = tel.id " );
  }

  where += QString( "opsub.opsub_type IN (%1) " ).arg( opsubType.join( ", " ) );

  QString query = QString( "SELECT DISTINCT opsub.id opsub_id "
                           "FROM opsub "
                           "%1 %2 "
                           "ORDER BY opsub.prijmeni, opsub.nazev;").arg( join ).arg( where );
  return evaluate( query );
}


bool VfkTableModel::searchPar( QString parcelniCislo, QString typIndex, QString druhKod, QString lv )
{
  QString where = "WHERE ";
  QString join;
  if ( !parcelniCislo.isEmpty() )
  {
    QStringList kmenAPoddeleni = parcelniCislo.split( "/" );
    where += QString( "par.kmenove_cislo_par = %1 AND " ).arg( kmenAPoddeleni.at( 0 ) );
    if ( kmenAPoddeleni.size() == 2 && !kmenAPoddeleni.at( 1 ).isEmpty())
    {
      where += QString( "par.poddeleni_cisla_par = %1 AND " ).arg( kmenAPoddeleni.at( 1 ) );
    }
  }
  if ( !druhKod.isEmpty() )
  {
    where += QString( "drupoz.kod = %1 AND " ).arg( druhKod );
  }

  if ( typIndex == "1" )
  {
    where += QString( "drupoz.stavebni_parcela = \"n\" AND " );
  }
  else if ( typIndex == "2" )
  {
    where += QString( "drupoz.stavebni_parcela = \"a\" AND " );
  }

  if ( !druhKod.isEmpty() )
  {
    where += QString( "par.drupoz_kod = %1 AND " ).arg( druhKod );
  }

  if ( !lv.isEmpty() )
  {
    where += QString( "tel.cislo_tel = %1 AND " ).arg( lv );
    join +=  "JOIN tel ON tel.id = par.tel_id ";
  }
  where += "1 ";

  QString query = QString( "SELECT DISTINCT par.id par_id "
                           "FROM par "
                           "JOIN drupoz ON par.drupoz_kod = drupoz.kod "
                           "%1 %2;").arg( join ).arg( where );
  return evaluate( query );
}

bool VfkTableModel::searchBud( QString domovniCislo, QString naParcele, QString zpusobVyuzitiKod, QString lv )
{
  QString where = "WHERE ";
  QString join;
  if ( !domovniCislo.isEmpty() )
  {
    where += QString( "bud.cislo_domovni = %1 AND " ).arg( domovniCislo );
  }

  if ( !naParcele.isEmpty() )
  {
    QStringList kmenAPoddeleni = naParcele.split( "/" );
    where += QString( "par.kmenove_cislo_par = %1 AND " ).arg( kmenAPoddeleni.at( 0 ) );
    if ( kmenAPoddeleni.size() == 2 && !kmenAPoddeleni.at( 1 ).isEmpty())
    {
      where += QString( "par.poddeleni_cisla_par = %1 AND " ).arg( kmenAPoddeleni.at( 1 ) );
    }
    join += "JOIN par ON bud.id = par.bud_id ";
  }

  if ( !lv.isEmpty() )
  {
    where += QString( "tel.cislo_tel = %1 AND " ).arg( lv );
    join += "JOIN tel ON tel.id = bud.tel_id ";
  }

  if ( !zpusobVyuzitiKod.isEmpty() )
  {
    where += QString( "zpvybu.kod = %1 AND " ).arg( zpusobVyuzitiKod );
    join += "JOIN zpvybu ON zpvybu.kod = bud.zpvybu_kod ";
  }

  where += "1 ";

  QString query = QString( "SELECT DISTINCT bud.id bud_id "
                           "FROM bud "
                           "%1 %2;").arg( join ).arg( where );
  return evaluate( query );
}

bool VfkTableModel::searchJed(QString cisloJednotky, QString domovniCislo, QString naParcele, QString zpusobVyuzitiKod, QString lv)
{
  QString where = "WHERE ";
  QString join;
  if ( !cisloJednotky.isEmpty() )
  {
    where += QString( "jed.cislo_jednotky = %1 AND " ).arg( cisloJednotky );
  }

  if ( !domovniCislo.isEmpty() )
  {
    where += QString( "bud.cislo_domovni = %1 AND " ).arg( domovniCislo );
  }

  if ( !naParcele.isEmpty() )
  {
    QStringList kmenAPoddeleni = naParcele.split( "/" );
    where += QString( "par.kmenove_cislo_par = %1 AND " ).arg( kmenAPoddeleni.at( 0 ) );
    if ( kmenAPoddeleni.size() == 2 && !kmenAPoddeleni.at( 1 ).isEmpty())
    {
      where += QString( "par.poddeleni_cisla_par = %1 AND " ).arg( kmenAPoddeleni.at( 1 ) );
    }
    join += "JOIN par ON bud.id = par.bud_id ";
  }

  if ( !lv.isEmpty() )
  {
    where += QString( "tel.cislo_tel = %1 AND " ).arg( lv );
    join += "JOIN tel ON tel.id = jed.tel_id ";
  }

  if ( !zpusobVyuzitiKod.isEmpty() )
  {
    where += QString( "zpvyje.kod = %1 AND " ).arg( zpusobVyuzitiKod );
    join += "JOIN zpvyje ON zpvyje.kod = jed.zpvyje_kod ";
  }

  where += "1 ";

  QString query = QString( "SELECT DISTINCT jed.id jed_id "
                           "FROM jed "
                           "JOIN bud ON bud.id = jed.bud_id "
                           "%1 %2;").arg( join ).arg( where );
  return evaluate( query );
}

QStringList VfkTableModel::parColumns( bool extended )
{
  QStringList columns;
  columns << "par.id par_id" <<
             "par.kmenove_cislo_par par_kmenove_cislo_par" <<
             "par.poddeleni_cisla_par par_poddeleni_cisla_par" <<
             "par.vymera_parcely par_vymera_parcely" <<
             "tel.id tel_id" <<
             "tel.cislo_tel tel_cislo_tel" <<
             "drupoz.nazev drupoz_nazev" <<
             "drupoz.stavebni_parcela drupoz_stavebni_parcela" <<
             "zpvypo.nazev zpvypo_nazev" ;
  if ( extended )
  {
    columns << "par.stav_dat par_stav_dat" << "par.par_type par_par_type" <<
               "par.katuze_kod par_katuze_kod" << "katuze.nazev katuze_nazev" <<
               "maplis.oznaceni_mapoveho_listu maplis_oznaceni_mapoveho_listu" <<
               "zpurvy.nazev zpurvy_nazev" <<
               "par.cena_nemovitosti par_cena_nemovitosti" <<
               "bud_id bud_id" << "bud.cislo_domovni bud_cislo_domovni" <<
               "typbud.nazev typbud_nazev" <<
               "typbud.zkratka typbud_zkratka";
  }

  return columns;
}
QStringList VfkTableModel::budColumns( bool extended )
{
  QStringList columns;
  columns << "bud.id bud_id" <<
             "bud.typbud_kod bud_typbud_kod" <<
             "casobc.nazev casobc_nazev" <<
             "casobc.kod casobc_kod" <<
             "bud.cislo_domovni bud_cislo_domovni" <<
             "par.id par_id" <<
             "tel.id tel_id" <<
             "tel.cislo_tel tel_cislo_tel" <<
             "par.kmenove_cislo_par par_kmenove_cislo_par" <<
             "par.poddeleni_cisla_par par_poddeleni_cisla_par" <<
             "drupoz.stavebni_parcela drupoz_stavebni_parcela" <<
             "zpvybu.kod zpvybu_kod" <<
             "zpvybu.nazev zpvybu_nazev" <<
             "typbud.nazev typbud_nazev" <<
             "typbud.zkratka typbud_zkratka" <<
             "typbud.zadani_cd typbud_zadani_cd" ;
  if ( extended )
  {
    columns << "bud.cena_nemovitosti bud_cena_nemovitosti" <<
               "par.katuze_kod par_katuze_kod" <<
               "katuze.nazev katuze_nazev";
  }

  return columns;
}

QStringList VfkTableModel::jedColumns( bool extended )
{
  QStringList columns;
  columns << "jed.id jed_id" <<
             "bud.id bud_id" <<
             "bud.cislo_domovni bud_cislo_domovni" <<
             "typjed.nazev typjed_nazev" <<
             "jed.cislo_jednotky jed_cislo_jednotky" <<
             "zpvyje.nazev zpvyje_nazev" <<
             "jed.podil_citatel jed_podil_citatel" <<
             "jed.podil_jmenovatel jed_podil_jmenovatel"  <<
             "tel.id tel_id" <<
             "tel.cislo_tel tel_cislo_tel";

  if ( extended )
  {
    columns << "jed.cena_nemovitosti jed_cena_nemovitosti" <<
               "jed.popis jed_popis" <<
               "typbud.zkratka typbud_zkratka" <<
               "par.katuze_kod par_katuze_kod" <<
               "par.id par_id" <<
               "drupoz.stavebni_parcela drupoz_stavebni_parcela" <<
               "par.kmenove_cislo_par par_kmenove_cislo_par" <<
               "par.poddeleni_cisla_par par_poddeleni_cisla_par" <<
               "tel.id tel_id" <<
               "tel.cislo_tel tel_cislo_tel" <<
               "katuze.nazev katuze_nazev";
  }

  return columns;

}

QStringList VfkTableModel::opsubColumns( bool extended )
{
  QStringList columns;

  columns << "opsub.opsub_type opsub_opsub_type" <<
             "opsub.id opsub_id" <<
             "charos.zkratka charos_zkratka" <<
             "charos.nazev charos_nazev" <<
             "opsub.nazev opsub_nazev" <<
             "opsub.titul_pred_jmenem opsub_titul_pred_jmenem" <<
             "opsub.jmeno opsub_jmeno" <<
             "opsub.prijmeni opsub_prijmeni" <<
             "opsub.titul_za_jmenem opsub_titul_za_jmenem";

  if ( extended )
  {
    columns << "opsub.id_je_1_partner_bsm opsub_id_je_1_partner_bsm" <<
               "opsub.id_je_2_partner_bsm opsub_id_je_2_partner_bsm" <<
               "opsub.ico opsub_ico" <<
               "opsub.rodne_cislo opsub_rodne_cislo" <<
               "opsub.cislo_domovni opsub_cislo_domovni" <<
               "opsub.cislo_orientacni opsub_cislo_orientacni" <<
               "opsub.nazev_ulice opsub_nazev_ulice" <<
               "opsub.cast_obce opsub_cast_obce" <<
               "opsub.obec opsub_obec" <<
               "opsub.psc opsub_psc" <<
               "opsub.mestska_cast opsub_mestska_cast";
  }

  return columns;
}

QStringList VfkTableModel::jpvColumns( bool extended )
{
  QStringList columns;

  columns << "typrav.nazev typrav_nazev" <<
             "jpv.id jpv_id" <<
             "jpv.popis_pravniho_vztahu jpv_popis_pravniho_vztahu" <<
             "jpv.par_id_k jpv_par_id_k" <<
             "jpv.par_id_pro jpv_par_id_pro" <<
             "jpv.bud_id_k jpv_bud_id_k" <<
             "jpv.bud_id_pro jpv_bud_id_pro" <<
             "jpv.jed_id_k jpv_jed_id_k" <<
             "jpv.jed_id_pro jpv_jed_id_pro" <<
             "jpv.opsub_id_k jpv_opsub_id_k" <<
             "jpv.opsub_id_pro jpv_opsub_id_pro";

  if ( extended )
  {
  }

  return columns;
}

QStringList VfkTableModel::listinyColumns()
{
  QStringList columns;

  columns << "rl.listin_id rl_listin_id" <<
             "rl.opsub_id rl_opsub_id" <<
             "typlis.nazev typlis_nazev" <<
             "dul.nazev dul_nazev";
  return columns;
}

QStringList VfkTableModel::bpejColumns()
{
  QStringList columns;

  columns << "bdp.bpej_kod bdp_bpej_kod" <<
             "bdp.vymera bdp_vymera" <<
             "par.id par_id" <<
             "par.kmenove_cislo_par par_kmenove_cislo_par" <<
             "par.poddeleni_cisla_par par_poddeleni_cisla_par" <<
             "drupoz.stavebni_parcela drupoz_stavebni_parcela";
  return columns;
}

QString VfkTableModel::nemovitost2TableName( VfkTableModel::Nemovitost nemovitost )
{
  QString table;

  switch ( nemovitost )
  {
  case NParcela:
    table = "par";
    break;
  case NBudova:
    table = "bud";
    break;
  case NJednotka:
    table = "jed";
    break;
  default:
    break;
  }

  return table;
}

QString VfkTableModel::opravnenyPovinny2TableName( VfkTableModel::OpravnenyPovinny opravnenyPovinny )
{
  QString table;
  switch ( opravnenyPovinny )
  {
  case OPParcela:
    table = "par";
    break;
  case OPBudova:
    table = "bud";
    break;
  case OPJednotka:
    table = "jed";
    break;
  case OPOsoba:
    table = "opsub";
    break;
  default:
    break;
  }

  return table;
}

QString VfkTableModel::pravo2ColumnSuffix( VfkTableModel::Pravo pravo )
{
  QString columnNameSuffix;
  switch ( pravo )
  {
  case Opravneni:
    columnNameSuffix = "pro";
    break;
  case Povinnost:
    columnNameSuffix = "k";
    break;
  default:
    break;
  }
  return columnNameSuffix;
}

VfkTableModel::OpravnenyPovinny VfkTableModel::tableName2OpravnenyPovinny( QString name )
{
    if ( name.contains( "par" ) )
    {
      return VfkTableModel::OPParcela;
    }
    if ( name.contains( "bud" ) )
    {
      return VfkTableModel::OPBudova;
    }
    if ( name.contains( "jed" ) )
    {
      return VfkTableModel::OPJednotka;
    }
    if ( name.contains( "opsub" ) )
    {
      return VfkTableModel::OPOsoba;
    }
    return VfkTableModel::OPParcela;
}

bool VfkTableModel::druhyPozemku( bool pozemkova, bool stavebni )
{
  QString where;
  if ( pozemkova && stavebni )
  {
  }
  else if ( pozemkova )
  {
    where += "WHERE drupoz.stavebni_parcela = \"n\"";
  }
  else
  {
    where += "WHERE drupoz.stavebni_parcela = \"a\"";
  }
  QString query = QString( "SELECT drupoz.kod drupoz_kod, drupoz.zkratka drupoz_zkratka "
                           "FROM drupoz "
                           "%1;" ).arg( where );

  return evaluate( query );
}

bool VfkTableModel::zpusobVyuzitiBudov()
{
  QString query = QString( "SELECT zpvybu.kod zpvybu_kod, zpvybu.zkratka zpvybu_zkratka "
                           "FROM zpvybu; " );

  return evaluate( query );
}

bool VfkTableModel::zpusobVyuzitiJednotek()
{
  QString query = QString( "SELECT zpvyje.kod zpvyje_kod, zpvyje.zkratka zpvyje_zkratka "
                           "FROM zpvyje; " );

  return evaluate( query );
}

bool VfkTableModel::evaluate( QString query )
{
  QTime t;
  t.start();
  setQuery( query, QSqlDatabase::database( mConnectionName ) );

  while ( canFetchMore() )
  {
       fetchMore();
  }
  if ( t.elapsed() > 500)
  {
    qDebug() << query;
    qDebug("\nTime elapsed: %d ms\n", t.elapsed());
  }
  if ( lastError().isValid() )
  {
    qDebug() << lastError() << query;
    return false;
  }
  return true;
}
QString VfkTableModel::value( int row, QString column) const
{
  return record( row ).field( column ).value().toString();
}
