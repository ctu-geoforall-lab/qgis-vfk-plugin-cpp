#include <QDebug>
#include <QRegExp>
#include <QSqlError>
#include <QStringList>

#include "documentbuilder.h"
#include "domains.h"

typedef QMap<QString, TableContent> TableContentMap;

DocumentBuilder::DocumentBuilder():
  mHasConnection( false )
{
}

DocumentBuilder::DocumentBuilder( QString connectionName ):
  mHasConnection( true ),
  mConnectionName( connectionName ),
  mDveRadyCislovani( false )

{
  mStringBezZapisu = QObject::trUtf8( "Bez zápisu." );
  initKatUzemi();
}

DocumentBuilder::~DocumentBuilder()
{
}

void DocumentBuilder::buildHtml( VfkDocument *document, TaskMap taskMap )
{
  mCurrentPageParIds.clear();
  mCurrentPageBudIds.clear();
  mCurrentDefinitionPoint.first.clear();
  mCurrentDefinitionPoint.second.clear();

  mDocument = document;
  mDocument->header();


  if ( taskMap["page"] == "help" )
  {
    pageHelp();
  }

  if ( mHasConnection )
  {
    if (taskMap["page"] == "tel")
    {
      pageTeleso( taskMap["id"] );
    }
    else if (taskMap["page"] == "par")
    {
      pageParcela( taskMap["id"] );
    }
    else if (taskMap["page"] == "bud")
    {
      pageBudova( taskMap["id"] );
    }
    else if (taskMap["page"] == "jed")
    {
      pageJednotka( taskMap["id"] );
    }
    else if (taskMap["page"] == "opsub")
    {
      pageOpravnenySubjekt( taskMap["id"] );
    }
    else if (taskMap["page"] == "seznam") // FIXME
    {
      if ( taskMap[ "type" ] == "id" )
      {
        if ( taskMap.contains( "parcely" ) )
        {
          pageSeznamParcel( taskMap["parcely"].split( "," ) );
        }
        if ( taskMap.contains( "budovy" ) )
        {
          pageSeznamBudov( taskMap[ "budovy" ].split( "," ) );
        }
      }
      else if ( taskMap[ "type" ] == "string" )
      {
        if ( taskMap.contains( "opsub" ) )
        {
          //        pageSeznamOsob( taskMap["opsub"] );
        }
      }
    }
    else if ( taskMap[ "page" ] == "search" )
    {
      if ( taskMap[ "type" ] == "vlastnici" )
      {
        pageSearchVlastnici( taskMap[ "jmeno" ], taskMap[ "rcIco" ],
                             taskMap[ "sjm" ].toInt(), taskMap[ "opo" ].toInt(),
                             taskMap[ "ofo" ].toInt(), taskMap[ "lv" ] );
      }
      else if ( taskMap[ "type" ] == "parcely" )
      {
        pageSearchParcely( taskMap[ "parcelniCislo" ], taskMap[ "typ" ], taskMap[ "druh" ], taskMap[ "lv" ] );
      }
      else if ( taskMap[ "type" ] == "budovy" )
      {
        pageSearchBudovy( taskMap[ "domovniCislo" ], taskMap[ "naParcele" ],
                          taskMap[ "zpusobVyuziti" ], taskMap[ "lv" ] );
      }
      else if ( taskMap[ "type" ] == "jednotky" )
      {
        pageSearchJednotky( taskMap[ "cisloJednotky" ], taskMap[ "domovniCislo" ], taskMap[ "naParcele" ],
                            taskMap[ "zpusobVyuziti" ], taskMap[ "lv" ] );
      }
    }
  }
  mDocument->footer();
  return;
}

void DocumentBuilder::initKatUzemi()
{
  VfkTableModel model( mConnectionName );
  if ( model.dveRadyCislovani() )
  {
    mDveRadyCislovani = true;
  }

}

void DocumentBuilder::pageTelesa()
{
  VfkTableModel model( mConnectionName );
  bool ok = model.telesa();
  if ( !ok )
  {
    return; // FIXME
  }

  for ( int i = 0; i < model.rowCount(); i++ )
  {
    QString tel_id = model.value( i, "tel_id" );
    QString cislo_tel =  model.value( i, "tel_cislo_tel" );
    QString link = mDocument->link( QString( "showText?page=tel&id=%1" ).arg( tel_id ), cislo_tel + "<br/>" );
    mDocument->text( link );
  }
}

void DocumentBuilder::pageTeleso( QString id )
{
  QStringList parIds, budIds, jedIds, opsubIds;
  partTelesoHlavicka( id );
  partTelesoVlastnici( id, opsubIds, true );
  partTelesoNemovitosti( id, parIds, budIds, jedIds );

  partTelesoB1( parIds, budIds, jedIds, opsubIds, true );
  partTelesoC( parIds, budIds, jedIds, opsubIds, true );
  partTelesoD( parIds, budIds, jedIds, opsubIds, true );
  partTelesoE( parIds, budIds, jedIds );
  partTelesoF( parIds, true );
}

void DocumentBuilder::partTelesoHlavicka( QString id )
{
  VfkTableModel hlavickaModel( mConnectionName );
  bool ok = hlavickaModel.telesoHlavicka( id );
  if ( !ok )
  {
    return; //FIXME
  }

  mDocument->heading1( QObject::trUtf8( "List vlastnictví" ) );
  KeyValList content;

  content.append( qMakePair( QObject::trUtf8( "List vlastnictví:" ), makeLVCislo( &hlavickaModel, 0 ) ) );
  content.append( qMakePair( QObject::trUtf8( "Kat. území:" ), makeKatastrUzemi( &hlavickaModel, 0 )) );
  content.append( qMakePair( QObject::trUtf8( "Obec:" ), makeObec( &hlavickaModel, 0 )) );
  content.append( qMakePair( QObject::trUtf8( "Okres:" ), QString( "%1 %2" )
                             .arg(hlavickaModel.value( 0, "okresy_nazev" ) )
                             .arg(hlavickaModel.value( 0, "okresy_nuts4" ) ) ) );
  mDocument->keyValueTable( content );

  if ( hlavickaModel.dveRadyCislovani() )
  {
    mDocument->paragraph( QObject::trUtf8( "V kat. území jsou pozemky vedeny ve dvou číselných řadách." ) );
  }
  else
  {
    mDocument->paragraph( QObject::trUtf8( "V kat. území jsou pozemky vedeny v jedné číselné řadě." ) );
  }

}

void DocumentBuilder::partTelesoNemovitosti( QString id, QStringList &parIds,
                                             QStringList &budIds, QStringList &jedIds  )
{
  mDocument->heading2( QObject::trUtf8( "B – Nemovitosti" ) );
  partTelesoParcely( id, parIds );
  partTelesoBudovy( id, budIds );
  partTelesoJednotky( id, jedIds );

  mCurrentPageParIds = parIds;
  mCurrentPageBudIds = budIds;
}

void DocumentBuilder::partVlastnikNemovitosti( QString opsubId )
{
  mDocument->heading2( QObject::trUtf8( "Nemovitosti vlastníka" ) );
  partVlastnikParcely( opsubId );
  partVlastnikBudovy( opsubId );
  partVlastnikJednotky( opsubId );
}

void DocumentBuilder::partTelesoParcely( QString opsubId, QStringList &parIds )
{

  VfkTableModel model( mConnectionName );
  bool ok = model.telesoParcely( opsubId, false );
  if ( !ok || model.rowCount() == 0 )
  {
    return;
  }
  tableParcely( &model, parIds, false );
}

void DocumentBuilder::partVlastnikParcely( QString id )
{

  VfkTableModel model( mConnectionName );
  bool ok = model.vlastnikParcely( id, false );
  if ( !ok || model.rowCount() == 0 )
  {
    return;
  }
  QStringList parIds;
  tableParcely( &model, parIds, true );
  mCurrentPageParIds = parIds;
}

void DocumentBuilder::tableParcely( const VfkTableModel *model, QStringList &parIds, bool LVcolumn )
{
  mDocument->heading3( QObject::trUtf8( "Pozemky" ) );
  mDocument->beginTable();
  QStringList header;
  header << QObject::trUtf8( "Parcela" )
         << QObject::trUtf8( "Výměra [m%1]" ).arg( mDocument->superscript( "2" ) )
         << QObject::trUtf8( "Druh pozemku" ) << QObject::trUtf8( "Způsob využití" )
         << QObject::trUtf8( "Způsob ochrany" );
  if ( LVcolumn )
  {
    header << QObject::trUtf8( "LV" );
  }
  mDocument->tableHeader( header );

  for ( int i = 0; i < model->rowCount(); i++ )
  {
    QStringList row;

    row << makeParcelniCislo( model, i );
    row << model->value( i, "par_vymera_parcely" );
    row << model->value( i, "drupoz_nazev" );
    row << model->value( i, "zpvypo_nazev" );

    QString parcelaId = model->value( i, "par_id" );
    VfkTableModel ochranaModel( mConnectionName );

    bool ok = ochranaModel.nemovitostOchrana( parcelaId, VfkTableModel::NParcela );
    if ( !ok )
    {
      break;
    }
    QStringList ochranaNazev;
    for ( int j = 0; j < ochranaModel.rowCount(); j++ )
    {
      ochranaNazev.append( ochranaModel.value( j, "zpochn_nazev") );
    }

    row << ochranaNazev.join( ", " );

    if ( LVcolumn )
    {
      row << makeLVCislo( model, i );
    }

    mDocument->tableRow( row );

    // ids of all parcels needed for jpv (B1, C, D)
    parIds << parcelaId;

  }
  mDocument->endTable();
}

void DocumentBuilder::partTelesoBudovy(QString opsubId, QStringList &budIds )
{

  VfkTableModel model( mConnectionName );
  bool ok = model.telesoBudovy( opsubId, false );
  if ( !ok || model.rowCount() == 0 )
  {
    return;
  }

  tableBudovy( &model, budIds, false );
}

void DocumentBuilder::partVlastnikBudovy( QString id )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.vlastnikBudovy( id, false );
  if ( !ok || model.rowCount() == 0 )
  {
    return;
  }
  QStringList budIds;
  tableBudovy( &model, budIds, true );
  mCurrentPageBudIds << budIds;
}

void DocumentBuilder::tableBudovy( const VfkTableModel *model, QStringList &budIds, bool LVcolumn )
{

  mDocument->heading3( QObject::trUtf8( "Stavby" ) );
  mDocument->beginTable();
  QStringList header;
  header << QObject::trUtf8( "Typ stavby" )
         << QObject::trUtf8( "Část obce" )
         << QObject::trUtf8( "Č. budovy" )
         << QObject::trUtf8( "Způsob využití" )
         << QObject::trUtf8( "Způsob ochrany" )
         << QObject::trUtf8( "Na parcele" ) ;
  if ( LVcolumn )
  {
    header << QObject::trUtf8( "LV" );
  }

  mDocument->tableHeader( header );

  for ( int i = 0; i < model->rowCount(); i++ )
  {
    QStringList row;
    if ( !Domains::anoNe( model->value( i, "typbud_zadani_cd" ) ) )
    {
      row <<  mDocument->link( QString( "showText?page=bud&id=%1" ).arg( model->value( i, "bud_id" ) ),
                               model->value( i, "typbud_zkratka" ) );
      row << model->value( i, "casobc_nazev" );
      row << "";
    }
    else
    {
      row << "";
      row << model->value( i, "casobc_nazev" );
      row << mDocument->link( QString( "showText?page=bud&id=%1" ).arg( model->value( i, "bud_id" ) ),
                              QString( "%1 %2" ).arg( model->value( i, "typbud_zkratka" ) )
                              .arg( model->value( i, "bud_cislo_domovni" ) ) );
    }

    row << model->value( i, "zpvybu_nazev" );


    QString budId = model->value( i, "bud_id" );
    VfkTableModel ochranaModel( mConnectionName );

    bool ok = ochranaModel.nemovitostOchrana( budId, VfkTableModel::NBudova );
    if ( !ok )
    {
      break;
    }
    QStringList ochranaNazev;
    for ( int j = 0; j < ochranaModel.rowCount(); j++ )
    {
      ochranaNazev.append( ochranaModel.value( j, "zpochn_nazev") );
    }

    row << ochranaNazev.join( ", " );
    row << makeParcelniCislo( model, i );
    if ( LVcolumn )
    {
      row << makeLVCislo( model, i );
    }

    mDocument->tableRow( row );

    // ids of all parcels needed for jpv (B1, C, D)
    budIds << budId;
  }
  mDocument->endTable();
  // FIXME bytovy dum ?
}

void DocumentBuilder::partTelesoJednotky( QString id, QStringList &jedIds )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.telesoJednotky(id, false );
  if ( !ok || model.rowCount() == 0 )
  {
    return;
  }
  tableJednotky( &model, jedIds, false );

}

void DocumentBuilder::partVlastnikJednotky( QString opsubId )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.vlastnikJednotky( opsubId, false );
  if ( !ok || model.rowCount() == 0 )
  {
    return;
  }
  QStringList jedIds;
  tableJednotky( &model, jedIds, true );
}

void DocumentBuilder::tableJednotky( const VfkTableModel *model, QStringList &jedIds, bool LVcolumn )
{
  mDocument->heading3( QObject::trUtf8( "Jednotky" ) );
  mDocument->beginTable();
  QStringList header;
  header << QObject::trUtf8( "Č.p./Č.jednotky " )
         << QObject::trUtf8( "Způsob využití" )
         << QObject::trUtf8( "Způsob ochrany" )
         << QObject::trUtf8( "Podíl na společných%1částech domu a pozemku" ).arg( mDocument->newLine() ) ;

  if ( LVcolumn )
  {
    header << QObject::trUtf8( "LV" );
  }
  mDocument->tableHeader( header );

  for ( int i = 0; i < model->rowCount(); i++ )
  {
    QStringList row;

    QString jedId = model->value( i, "jed_id" );
    row << makeJednotka( model, i );
    row << model->value( i, "zpvyje_nazev");
    VfkTableModel ochranaModel( mConnectionName );

    bool ok = ochranaModel.nemovitostOchrana( jedId, VfkTableModel::NJednotka );
    if ( !ok )
    {
      break;
    }
    QStringList ochranaNazev;
    for ( int j = 0; j < ochranaModel.rowCount(); j++ )
    {
      ochranaNazev.append( ochranaModel.value( j, "zpochn_nazev") );
    }

    row << ochranaNazev.join( ", " );

    QString podilCit = model->value( i, "jed_podil_citatel" );
    QString podilJmen = model->value( i, "jed_podil_jmenovatel" );
    QString podil = "";
    if ( !podilCit.isEmpty() && !podilJmen.isEmpty()
         && podilJmen != "1" )
    {
      podil += QString("%1/%2").arg( podilCit ).arg( podilJmen );
    }

    row << podil;

    if ( LVcolumn )
    {
      row << makeLVCislo( model, i );
    }

    mDocument->tableRow( row );
    partTelesoJednotkaDetail( model->value( i, "bud_id" ) );

    // ids of all parcels needed for jpv (B1, C, D)
    jedIds << jedId;
  }
  mDocument->endTable();
}

void DocumentBuilder::partTelesoJednotkaDetail( QString budId )
{
  QString budInfo;
  QString parInfo;

  // details about budova
  VfkTableModel budModel( mConnectionName );
  bool ok = budModel.budova( budId, false );
  if ( !ok || budModel.rowCount() == 0 )
  {
    return;
  }

  budInfo += QObject::trUtf8( "Budova" ) + " ";
  QString casobc = budModel.value( 0, "casobc_nazev" );
  budInfo += casobc.isEmpty() ? "" : casobc + ", ";

  QString budova;
  budova += budModel.value( 0, "typbud_zkratka" );
  if ( Domains::anoNe( budModel.value( 0, "typbud_zadani_cd" ) ) )
  {
    budova += " " + budModel.value( 0, "bud_cislo_domovni" );
  }
  budInfo += mDocument->link( QString( "showText?page=bud&id=%1" ).arg( budId ), budova );

  QString lv = budModel.value( 0, "tel_cislo_tel" );
  QString lvId = budModel.value( 0, "tel_id" );
  if ( !lv.isEmpty() )
  {
    budInfo += mDocument->link( QString( "showText?page=tel&id=%1" ).arg( lvId ), QString( "LV %1" ).arg( lv ) );
  }

  QString zpvybu = budModel.value( 0, "zpvybu_nazev" );
  budInfo += zpvybu.isEmpty() ? "" : QString( ", %1").arg( zpvybu );

  budInfo += QObject::trUtf8( ", na parcele %1" )
      .arg( makeParcelniCislo( &budModel, 0 ) );

  mDocument->tableRowOneColumnSpan( budInfo );

  // details about parcela
  QString parcelaId = budModel.value( 0, "par_id" );
  VfkTableModel parModel( mConnectionName );
  ok = parModel.parcela( parcelaId, false );
  if ( !ok )
  {
    return; //FIXME
  }

  parInfo += QObject::trUtf8( "Parcela %1" ).arg( makeParcelniCislo( &parModel, 0 ) );
  lv = parModel.value( 0, "tel_cislo_tel" );
  lvId = parModel.value( 0, "tel_id" );
  if ( !lv.isEmpty() )
  {
    parInfo += mDocument->link( QString( "showText?page=tel&id=%1" ).arg( lvId ), QString( "LV %1" ).arg( lv ) );
  }
  QString zpvypo = parModel.value( 0, "zpvypo_nazev" );
  parInfo += zpvypo.isEmpty() ? "" : QString( ", %1" ).arg( zpvypo );
  parInfo += QString( ", %1 m%2" ).arg( parModel.value( 0, "par_vymera_parcely" ) )
      .arg( mDocument->superscript( "2" ) );

//  text += QString( "<tr><td colspan=\"4\">%1</td></tr>" ).arg( parInfo );
  mDocument->tableRowOneColumnSpan( parInfo );
}

void DocumentBuilder::partTelesoB1( const QStringList &parIds, const QStringList &budIds,
                                const QStringList &jedIds, const QStringList &opsubIds,
                                bool forLV )
{
  QStringList header;
  header << QObject::trUtf8( "Typ vztahu" )
         << QObject::trUtf8( "Oprávnění pro" )
         << QObject::trUtf8( "Povinnost k" );

  if ( forLV )
  {
    mDocument->heading2( QObject::trUtf8( "B1 – Jiná práva" ) );
    mDocument->beginTable();
    mDocument->tableHeader( header );

    if( partTelesoJinaPrava( parIds, VfkTableModel::OPParcela ) ||
        partTelesoJinaPrava( budIds, VfkTableModel::OPBudova) ||
        partTelesoJinaPrava( jedIds, VfkTableModel::OPJednotka ) ||
        partTelesoJinaPrava( opsubIds, VfkTableModel::OPOsoba ) )
    {
      mDocument->endTable();
    }
    else
    {
      mDocument->discardLastBeginTable();
      mDocument->text( mStringBezZapisu );
    }
  }
  else
  {
    mDocument->heading2( QObject::trUtf8( "Jiná práva" ) );
    mDocument->beginTable();
    mDocument->tableHeader( header );

    if ( partNemovitostJinaPrava( parIds, VfkTableModel::OPParcela ) ||
         partNemovitostJinaPrava( budIds, VfkTableModel::OPBudova) ||
         partNemovitostJinaPrava( jedIds, VfkTableModel::OPJednotka ) ||
         partNemovitostJinaPrava( opsubIds, VfkTableModel::OPOsoba ) )
    {
      mDocument->endTable();
    }
    else
    {
      mDocument->discardLastBeginTable();
      mDocument->text( mStringBezZapisu );
    }
  }
}
void DocumentBuilder::partTelesoC( const QStringList &parIds, const QStringList &budIds,
                               const QStringList &jedIds, const QStringList &opsubIds,
                               bool forLV )
{
  QStringList header;
  header << QObject::trUtf8( "Typ vztahu" )
         << QObject::trUtf8( "Oprávnění pro" )
         << QObject::trUtf8( "Povinnost k" );

  if ( forLV )
  {
    mDocument->heading2( QObject::trUtf8( "C – Omezení vlastnického práva" ) );
    mDocument->beginTable();
    mDocument->tableHeader( header );
    if ( partTelesoOmezeniPrava( parIds, VfkTableModel::OPParcela ) ||
         partTelesoOmezeniPrava( budIds, VfkTableModel::OPBudova ) ||
         partTelesoOmezeniPrava( jedIds, VfkTableModel::OPJednotka ) ||
         partTelesoOmezeniPrava( opsubIds, VfkTableModel::OPOsoba ) )
    {
      mDocument->endTable();
    }
    else
    {
      mDocument->discardLastBeginTable();
      mDocument->text( mStringBezZapisu );
    }

  }
  else
  {
    mDocument->heading2( QObject::trUtf8( "Omezení vlastnického práva" ) );
    mDocument->beginTable();
    mDocument->tableHeader( header );

    if( partNemovitostOmezeniPrava( parIds, VfkTableModel::OPParcela ) ||
        partNemovitostOmezeniPrava( budIds, VfkTableModel::OPBudova ) ||
        partNemovitostOmezeniPrava( jedIds, VfkTableModel::OPJednotka ) ||
        partNemovitostOmezeniPrava( opsubIds, VfkTableModel::OPOsoba ) )
    {
      mDocument->endTable();
    }
    else
    {
      mDocument->discardLastBeginTable();
      mDocument->text( mStringBezZapisu );
    }
  }
}

void DocumentBuilder::partTelesoD( const QStringList &parIds, const QStringList &budIds,
                               const QStringList &jedIds, const QStringList &opsubIds,
                               bool forLV )
{
  QStringList header;
  header << QObject::trUtf8( "Typ vztahu" )
         << QObject::trUtf8( "Vztah pro" )
         << QObject::trUtf8( "Vztah k" );

  if ( forLV )
  {
     mDocument->heading2( QObject::trUtf8( "D – Jiné zápisy" ) );
     mDocument->beginTable();
     mDocument->tableHeader( header );

     if ( partTelesoJineZapisy( parIds, VfkTableModel::OPParcela ) ||
          partTelesoJineZapisy( budIds, VfkTableModel::OPBudova ) ||
          partTelesoJineZapisy( jedIds, VfkTableModel::OPJednotka ) ||
          partTelesoJineZapisy( opsubIds, VfkTableModel::OPOsoba ) )
     {
       mDocument->endTable();
     }
     else
     {
       mDocument->discardLastBeginTable();
       mDocument->text( mStringBezZapisu );
     }
  }
  else
  {
     mDocument->heading2( QObject::trUtf8( "Jiné zápisy" ) );
     mDocument->beginTable();
     mDocument->tableHeader( header );

     if( partNemovitostJineZapisy( parIds, VfkTableModel::OPParcela ) ||
         partNemovitostJineZapisy( budIds, VfkTableModel::OPBudova ) ||
         partNemovitostJineZapisy( jedIds, VfkTableModel::OPJednotka ) ||
         partNemovitostJineZapisy( opsubIds, VfkTableModel::OPOsoba ) )
     {
       mDocument->endTable();
     }
     else
     {
       mDocument->discardLastBeginTable();
       mDocument->text( mStringBezZapisu );
     }
  }
}

void DocumentBuilder::partTelesoE( const QStringList &parIds,
                               const QStringList &budIds,
                               const QStringList &jedIds )
{
  mDocument->heading2( QObject::trUtf8( "E – Nabývací tituly a jiné podklady k zápisu" ) );

  VfkTableModel model( mConnectionName );
  bool ok = model.nabyvaciListiny( parIds, budIds, jedIds );
  if ( !ok )
  {
    return; //FIXME
  }
  if ( model.rowCount() == 0 )
  {
    mDocument->text( mStringBezZapisu );
  }
  else
  {
    QString lastListinaId;
    mDocument->beginItemize();
    for ( int i = 0; i < model.rowCount(); i++ )
    {
      QString currentListinaId = model.value( i, "rl_listin_id" );
      if ( currentListinaId == lastListinaId )
      {
        mDocument->item( makeShortDescription( model.value( i, "rl_opsub_id" ), VfkTableModel::OPOsoba ) );
      }
      else
      {
        if ( !lastListinaId.isEmpty() )
        {
          mDocument->endItemize();
          mDocument->endItem();

        }
        lastListinaId = currentListinaId;
        mDocument->beginItem();
        mDocument->text( makeListina( &model, i ) );
        mDocument->beginItemize();
        mDocument->item( makeShortDescription( model.value( i, "rl_opsub_id" ), VfkTableModel::OPOsoba ) );
      }
    }
    if ( !lastListinaId.isEmpty() )
    {
      mDocument->endItemize();
      mDocument->endItem();
    }
    mDocument->endItemize();

  }

//  return text;
}

void DocumentBuilder::partTelesoF( const QStringList &parIds, bool forLV )
{
  if ( forLV )
  {
    mDocument->heading2( QObject::trUtf8( "F – Vztah bonitovaných půdně ekologických "
                                          "jednotek (BPEJ) k parcelám" ) );
  }
  else
  {
    mDocument->heading2( QObject::trUtf8( "BPEJ" ) );

  }

  QStringList header;
  header << QObject::trUtf8( "Parcela" )
         << QObject::trUtf8( "BPEJ" )
         << QObject::trUtf8( "Výměra [m%1]" ).arg( mDocument->superscript( "2" ) );

  mDocument->beginTable();
  mDocument->tableHeader( header );

  bool isRecord = false;
  foreach( QString id, parIds )
  {
    QStringList row;
    VfkTableModel model( mConnectionName );
    bool ok = model.parcelaBpej( id );
    if ( !ok )
    {
      break;
    }
    if ( model.rowCount() == 0 )
    {
      continue;
    }
    isRecord = true;
    row << makeParcelniCislo( &model, 0 );
    row << model.value( 0, "bdp_bpej_kod" );
    row << model.value( 0, "bdp_vymera" );

    mDocument->tableRow( row );
  }

  if ( isRecord )
  {
    mDocument->endTable();
  }
  else
  {
    mDocument->discardLastBeginTable();
    mDocument->text( mStringBezZapisu );
  }
}

bool DocumentBuilder::partNemovitostJinaPrava(const QStringList ids, VfkTableModel::OpravnenyPovinny opravneny)
{
  return partTelesoB1CDSubjekt( ids, opravneny, VfkTableModel::Opravneni, false, false);
}

bool DocumentBuilder::partTelesoJinaPrava(const QStringList ids, VfkTableModel::OpravnenyPovinny opravneny)
{
  return partTelesoB1CDSubjekt( ids, opravneny, VfkTableModel::Opravneni, false, true);
}

bool DocumentBuilder::partNemovitostOmezeniPrava(const QStringList ids, VfkTableModel::OpravnenyPovinny povinny)
{
  return partTelesoB1CDSubjekt( ids, povinny, VfkTableModel::Povinnost, false, false );
}

bool DocumentBuilder::partTelesoOmezeniPrava(const QStringList ids, VfkTableModel::OpravnenyPovinny povinny)
{
  return partTelesoB1CDSubjekt( ids, povinny, VfkTableModel::Povinnost, false, true );
}

bool DocumentBuilder::partNemovitostJineZapisy(const QStringList ids, VfkTableModel::OpravnenyPovinny povinny )
{
  bool test1 = partTelesoB1CDSubjekt( ids, povinny, VfkTableModel::Opravneni, true, false );
  bool test2 = partTelesoB1CDSubjekt( ids, povinny, VfkTableModel::Povinnost, true, false );
  return ( test1 || test2 );
}

bool DocumentBuilder::partTelesoJineZapisy(const QStringList ids, VfkTableModel::OpravnenyPovinny povinny )
{
  bool test1 = partTelesoB1CDSubjekt( ids, povinny, VfkTableModel::Opravneni, true, true );
  bool test2 = partTelesoB1CDSubjekt( ids, povinny, VfkTableModel::Povinnost, true, true );
  return ( test1 || test2 );
}

bool DocumentBuilder::partTelesoB1CDSubjekt( const QStringList ids,
                                            VfkTableModel::OpravnenyPovinny pravniSubjekt,
                                            VfkTableModel::Pravo pravo,
                                            bool sekceD, bool showListiny )
{
  bool isRecord = false;
  QStringList povinni, opravneni;
  povinni << "jpv_par_id_k" << "jpv_bud_id_k" << "jpv_jed_id_k" << "jpv_opsub_id_k";
  opravneni << "jpv_par_id_pro" << "jpv_bud_id_pro" << "jpv_jed_id_pro" << "jpv_opsub_id_pro";

  foreach ( QString id, ids )
  {
    VfkTableModel model( mConnectionName );
    QString where = QString( "typrav.sekce %1= \"D\"" ).arg( sekceD ? "" : "!" );
    bool ok = model.nemovitostJpv( id, pravniSubjekt, pravo, where );
    if ( !ok || model.rowCount() == 0 )
    {
      continue;
    }
    isRecord = true;
    for ( int i = 0; i < model.rowCount(); i++ )
    {
      QStringList row;
      QString typPrava = model.value( i, "typrav_nazev" );
      row << typPrava;

      QStringList opravneniList;
      foreach ( QString column1, opravneni )
      {
        if ( !model.value( i, column1 ).isEmpty() )
        {
          VfkTableModel::OpravnenyPovinny opravneny = VfkTableModel::tableName2OpravnenyPovinny( column1 );
          QString opravnenyId = model.value( i, column1 );
          opravneniList << makeShortDescription( opravnenyId, opravneny );
        }
      }
      row << opravneniList.join( mDocument->newLine() );

      QStringList povinniList;
      foreach ( QString column2, povinni )
      {
        if ( !model.value( i, column2 ).isEmpty() )
        {
          VfkTableModel::OpravnenyPovinny povinny = VfkTableModel::tableName2OpravnenyPovinny( column2 );
          QString povinnyId = model.value( i, column2 );
          povinniList << makeShortDescription( povinnyId, povinny );
        }
      }
      row << povinniList.join( mDocument->newLine() );

      mDocument->tableRow( row );

      if ( showListiny )
      {
        partTelesoListiny( model.value( i, "jpv_id" ) );
      }
    }
  }
  return isRecord;
}


void DocumentBuilder::partTelesoListiny( QString jpvId )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.jpvListiny( jpvId );
  if ( !ok )
  {
    return; //FIXME
  }

  for( int i = 0; i < model.rowCount(); i++ )
  {
    mDocument->tableRowOneColumnSpan( makeListina( &model, i ) );
//    text += QString( "<tr><td colspan=\"3\">%1</td></tr>" ).arg( listina );
  }
}

void DocumentBuilder::pageParcela( QString id )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.parcela( id, true );
  if ( !ok )
  {
    return; //FIXME
  }
  mCurrentPageParIds << id;
  saveDefinitionPoint( id, VfkTableModel::NParcela );

  KeyValList content;

  content.append( qMakePair( QObject::trUtf8( "Parcelní číslo:" ), makeParcelniCislo( &model, 0 ) ) );

  VfkTableModel telesoModel( mConnectionName );
  telesoModel.nemovitostTeleso( id, VfkTableModel::NParcela );
  content.append( qMakePair( QObject::trUtf8( "List vlastnictví:" ),
                             makeLVCislo( &telesoModel, 0 ) ) );

  content.append( qMakePair( QObject::trUtf8( "Výměra [m%1]:" ).arg( mDocument->superscript( "2" ) ),
                             model.value( 0, "par_vymera_parcely" ) ) );

  content.append( qMakePair( QObject::trUtf8( "Určení výměry:" ),
                             model.value( 0, "zpurvy_nazev" ) ) );

  if ( !model.value( 0, "par_cena_nemovitosti" ).isEmpty() )
  {
    content.append( qMakePair( QObject::trUtf8( "Cena nemovitosti:" ),
                               model.value( 0, "par_cena_nemovitosti" ) ) );
  }

  content.append( qMakePair( QObject::trUtf8( "Typ parcely:" ), model.value( 0, "par_par_type" ) ) );

  content.append( qMakePair( QObject::trUtf8( "Mapový list:" ),
                             model.value( 0, "maplis_oznaceni_mapoveho_listu" ) ) );

  content.append( qMakePair( QObject::trUtf8( "Katastrální území:" ),
                             makeKatastrUzemi( &model, 0 ) ) );

  content.append( qMakePair( QObject::trUtf8( "Druh pozemku:" ), model.value( 0, "drupoz_nazev" ) ) );

  if ( !model.value( 0, "zpvypo_nazev" ).isEmpty() )
  {
    content.append( qMakePair( QObject::trUtf8( "Způsob využití pozemku:" ), model.value( 0, "zpvypo_nazev" ) ) );
  }

  if ( Domains::anoNe( model.value( 0, "drupoz_stavebni_parcela" ) ) )
  {
    content.append( qMakePair( QObject::trUtf8( "Stavba na parcele:" ), makeDomovniCislo( &model, 0 ) ) );
    mCurrentPageBudIds << model.value( 0, "bud_id" );
  }

  mDocument->heading1( QObject::trUtf8( "Informace o parcele" ) );
  mDocument->keyValueTable( content );


  // neighbours
  VfkTableModel sousedniModel( mConnectionName );
  sousedniModel.sousedniParcely( id );
  QStringList ids;
  for ( int i = 0; i < sousedniModel.rowCount(); i++ )
  {
    ids << sousedniModel.value( i, "hp_par_id_1" ) << sousedniModel.value( i, "hp_par_id_2" );
  }
  ids.removeDuplicates();
  ids.removeOne( id );
  QString link = mDocument->link( QString( "showText?page=seznam&type=id&parcely=%1" ).arg( ids.join( "," ) ),
                                  QObject::trUtf8( "Sousední parcely" ) );
  mDocument->paragraph( link );


//  text += QObject::trUtf8( "<a href=\"showText?page=seznam&type=id&par=%1\">Sousední parcely</a>" )
//      .arg( ids.join( "," ) );


  QStringList opsubIds; // not used
  partTelesoVlastnici( telesoModel.value( 0, "tel_id" ), opsubIds, false );

  partNemovitostOchrana( id, VfkTableModel::NParcela );
  QStringList parIds = QStringList() << id;
  partTelesoB1( parIds, QStringList(), QStringList(), QStringList(), false );
  partTelesoC( parIds, QStringList(), QStringList(), QStringList(), false );
  partTelesoD( parIds, QStringList(), QStringList(), QStringList(), false );
  partTelesoF( parIds, false );
}

void DocumentBuilder::partTelesoVlastnici( QString id, QStringList &opsubIds, bool forLV )
{
  VfkTableModel vlastniciModel( mConnectionName );
  bool ok = vlastniciModel.telesoVlastnici( id );
  if ( !ok )
  {
    return;
  }
  if ( forLV )
  {
    mDocument->heading2( QObject::trUtf8( "A – Vlastníci, jiní oprávnění" ) );
  }
  else
  {
    mDocument->heading2( QObject::trUtf8( "Vlastníci, jiní oprávnění" ) );
  }

  // find distinct values of typrav
  QStringList orderedPrava;

  for ( int i = 0; i < vlastniciModel.rowCount(); i++ )
  {
    orderedPrava << vlastniciModel.value( i, "typrav_nazev" );
  }
  orderedPrava.removeDuplicates();

  // create separate tables for different typrav (just headers)
  TableContentMap tables;
  QStringList header;
  header << QObject::trUtf8("Jméno") << QObject::trUtf8("Adresa")
         << QObject::trUtf8("Identifikátor") << QObject::trUtf8("Podíl");
  for ( int i = 0; i < orderedPrava.count(); i++ )
  {
    TableContent table;
    table.append( header );
//    QString table = QString( "<h3>%1</h3>" ).arg( orderedPrava.at( i ) );
//    table += QString( "<table class=\"%1table\">" ).arg( prefix );

//    table += makeTableHeader( header, prefix );
    tables.insert( orderedPrava.at( i ), table );
  }

  for ( int i = 0; i < vlastniciModel.rowCount(); i++ )
  {
    QString typravNazev = vlastniciModel.value( i, "typrav_nazev" );
    QString opsubId = vlastniciModel.value( i, "vla_opsub_id" );
    QString vlaPodilCitatel = vlastniciModel.value( i, "vla_podil_citatel" );
    QString vlaPodilJmenovatel = vlastniciModel.value( i, "vla_podil_jmenovatel" );
    QString podil = "";
    if ( !vlaPodilCitatel.isEmpty() && !vlaPodilJmenovatel.isEmpty()
         && vlaPodilJmenovatel != "1" )
    {
      podil += QString("%1/%2").arg( vlaPodilCitatel ).arg( vlaPodilJmenovatel );
    }

    VfkTableModel vlastnikModel( mConnectionName );
    ok = vlastnikModel.vlastnik( opsubId, true );
    if ( !ok )
    {
      return;//FIXME
    }

    QString opsub_type = vlastnikModel.value( 0, "opsub_opsub_type" );
    QString nazev = makeJmeno( &vlastnikModel, 0 );

    if ( opsub_type != "BSM" )
    {
      QStringList content;
      QString adresa = makeAdresa( &vlastnikModel, 0 );
      QString identifikator = makeIdentifikator( &vlastnikModel, 0 );
      content << nazev << adresa << identifikator << podil;
      tables[ typravNazev ].append( content ) ;
    }
    else
    {
      nazev += QString( " (%1)" ).arg( vlastnikModel.value( 0, "charos_zkratka" ) );

      QStringList rowContent;
      rowContent << nazev << "" << "" << podil; // adresa, identifikator are empty
      tables[ typravNazev ].append( rowContent );


      QStringList manzeleId;
      manzeleId << vlastnikModel.value( 0, "opsub_id_je_1_partner_bsm" );
      manzeleId << vlastnikModel.value( 0, "opsub_id_je_2_partner_bsm" );

      VfkTableModel sjmModel( mConnectionName );
      for ( int j = 0; j < manzeleId.count(); j++ )
      {
        ok = sjmModel.vlastnik( manzeleId.at( j ) );
        if ( !ok )
        {
          break;
        }
        QString identifikatorSJM = sjmModel.value( 0, "opsub_rodne_cislo" );
        rowContent.clear();
        rowContent << makeJmeno( &sjmModel, 0 )
                   << makeAdresa( &sjmModel, 0 )
                   << identifikatorSJM << "";
        tables[ typravNazev ].append( rowContent );
      }
    }
    // ids of all persons needed for jpv (B1, C, D)
    opsubIds.append( opsubId );
  }


  for (int i = 0; i < orderedPrava.count(); i++ )
  {
    mDocument->heading3( orderedPrava.at( i ) );
    mDocument->table( tables.value( orderedPrava.at( i ) ),true );
  }
}
// not used
//QString HtmlBuilder::partParcelaBpej( QString id )
//{
//  VfkTableModel bpej( mConnectionName );
//  bool ok = bpej.parcelaBpej( id );
//  if ( !ok )
//  {
//    return "";
//  }
//  QString prefix = "";
//  QString text = QObject::trUtf8( "<h2>BPEJ</h2>" );
//  if ( bpej.rowCount() == 0 )
//  {
//    text += QObject::trUtf8( "Nejsou evidovány žádné BPEJ." );
//  }
//  else
//  {
//    text += QString( "<table class=\"%1\">" ).arg( prefix );
//    QStringList header, content;
//    header << QObject::trUtf8( "Kód BPEJ" ) << QObject::trUtf8( "Výměra" );
//    text += makeTableHeader( header, prefix );
//    for ( int i = 0; i < bpej.rowCount(); i++ )
//    {
//      content.append( bpej.value( i, "bdp_bpej_kod" ) );
//      content.append( bpej.value( i, "bdp_vymera" ) );
//      text += makeTableRow( content, prefix );
//    }
//    text += "</table>\n";
//  }

//  return text;
//}

void DocumentBuilder::partNemovitostOchrana( QString id, VfkTableModel::Nemovitost nemovitost )
{
  VfkTableModel ochrana( mConnectionName );
  bool ok = ochrana.nemovitostOchrana( id, nemovitost );
  if ( !ok )
  {
    return; //FIXME
  }
  mDocument->heading2( QObject::trUtf8( "Způsob ochrany nemovitosti" ) );

  if ( ochrana.rowCount() == 0 )
  {
    mDocument->text( QObject::trUtf8( "Není evidován žádný způsob ochrany." ) );
  }
  else
  {
    mDocument->beginTable();
    QStringList header;
    header << QObject::trUtf8( "Název" );
    //text += makeTableHeader( header, prefix );
    for ( int i = 0; i < ochrana.rowCount(); i++ )
    {
      QStringList content( ochrana.value( i, "zpochn_nazev" ) );
      mDocument->tableRow( content );
    }
    mDocument->endTable();
  }
}

// not used
//QString HtmlBuilder::partParcelaJpv( QString id, VfkTableModel::Pravo pravo )
//{
//  VfkTableModel jpv( mConnectionName );
//  bool ok = jpv.nemovitostJpv( id, VfkTableModel::OPParcela, pravo );
//  if ( !ok )
//  {
//    return "";
//  }
//  QString prefix = "";

//  QString text;
//  if ( pravo == VfkTableModel::Povinnost )
//  {
//    text = QObject::trUtf8( "<h2>Omezení vlastnického práva</h2>" );
//  }
//  else
//  {
//    text = QObject::trUtf8( "<h2>Jiná práva</h2>" );
//  }
//  if ( jpv.rowCount() == 0 )
//  {
//    if ( pravo == VfkTableModel::Povinnost )
//    {
//      text += QObject::trUtf8( "Není evidováno žádné omezení." );
//    }
//    else
//    {
//      text += QObject::trUtf8( "Nejsou evidována žádná jiná práva." );
//    }
//  }
//  else
//  {
//    text += QString( "<table class=\"%1\">" ).arg( prefix );
//    QStringList header;
//    header << QObject::trUtf8( "Typ" );
//    //text += makeTableHeader( header, prefix );
//    for ( int i = 0; i < jpv.rowCount(); i++ )
//    {
//      QStringList content( jpv.value( i, "typrav_nazev" ) );
//      text += makeTableRow( content, prefix );
//    }
//    text += "</table>\n";
//  }
//  return text;
//}

void DocumentBuilder::pageBudova( QString id )
{

  VfkTableModel model( mConnectionName );
  bool ok = model.budova( id, true );
  if ( !ok )
  {
    return; //FIXME
  }
  mCurrentPageBudIds << id;
  saveDefinitionPoint( id, VfkTableModel::NBudova );

  KeyValList content;

  if ( Domains::anoNe( model.value( 0, "typbud_zadani_cd" ) ) )
  {
    content.append( qMakePair( QObject::trUtf8( "Stavba:" ), makeDomovniCislo( &model, 0 ) ) );
    content.append( qMakePair( QObject::trUtf8( "Část obce:" ), makeCastObce( &model, 0 ) ) );
  }
  content.append( qMakePair( QObject::trUtf8( "Na parcele:" ), makeParcelniCislo( &model, 0 ) ) );
  mCurrentPageParIds << model.value( 0, "par_id" );

  VfkTableModel telesoModel( mConnectionName );
  telesoModel.nemovitostTeleso( id, VfkTableModel::NBudova );
  content.append( qMakePair( QObject::trUtf8( "List vlastnictví:" ),
                             makeLVCislo( &telesoModel, 0 ) ) );

  QString cena = model.value( 0, "bud_cena_nemovitosti" );
  if ( !cena.isEmpty() )
  {
    content.append( qMakePair( QObject::trUtf8( "Cena nemovitosti:" ), cena ) );
  }
  content.append( qMakePair( QObject::trUtf8( "Typ stavby:" ),
                             model.value( 0, "typbud_nazev" ) ) );
  content.append( qMakePair( QObject::trUtf8( "Způsob využití:" ),
                             model.value( 0, "zpvybu_nazev" ) ) );

    VfkTableModel jednotkyModel( mConnectionName );
    jednotkyModel.budovaJednotky( id );
    if ( jednotkyModel.rowCount() > 0 )
    {
      QStringList jednotky;
      for ( int i = 0; i < jednotkyModel.rowCount(); i++ )
      {
        jednotky << makeJednotka( &jednotkyModel, i );
      }
      content.append( qMakePair( QObject::trUtf8( "Jednotky v budově:" ),
                                 jednotky.join( ", " ) ) );
    }


  content.append( qMakePair( QObject::trUtf8( "Katastrální území:" ),
                             makeKatastrUzemi( &model, 0 ) ) );

  mDocument->heading1( QObject::trUtf8("Informace o stavbě") );
  mDocument->keyValueTable( content );

  QStringList opsubIds;
  partTelesoVlastnici( telesoModel.value( 0, "tel_id" ), opsubIds, false );

  partNemovitostOchrana( id, VfkTableModel::NBudova );


  QStringList budIds = QStringList() << id;
  partTelesoB1( QStringList(), budIds, QStringList(), QStringList(), false );
  partTelesoC( QStringList(), budIds, QStringList(), QStringList(), false );
  partTelesoD( QStringList(), budIds, QStringList(), QStringList(), false );
}

void DocumentBuilder::pageJednotka( QString id )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.jednotka( id, true );
  if ( !ok )
  {
    return; //FIXME
  }

  KeyValList content;

  content.append( qMakePair( QObject::trUtf8( "Číslo jednotky:" ), makeJednotka( &model, 0 ) ) );
  content.append( qMakePair( QObject::trUtf8( "V budově:" ), makeDomovniCislo( &model, 0 ) ) );
  content.append( qMakePair( QObject::trUtf8( "Na parcele:" ), makeParcelniCislo( &model, 0 ) ) );

  mCurrentPageParIds << model.value( 0, "par_id" );
  mCurrentPageBudIds << model.value( 0, "bud_id" );

//  content.append( qMakePair( QObject::trUtf8( "Na parcele:" ), makeParcelniCislo( &model, 0 ) ) );

  VfkTableModel telesoModel( mConnectionName );
  telesoModel.nemovitostTeleso( id, VfkTableModel::NJednotka );
  content.append( qMakePair( QObject::trUtf8( "List vlastnictví:" ),
                             makeLVCislo( &telesoModel, 0 ) ) );

  QString cena = model.value( 0, "jed_cena_nemovitosti" );
  if ( !cena.isEmpty() )
  {
    content.append( qMakePair( QObject::trUtf8( "Cena nemovitosti:" ), cena ) );
  }
  content.append( qMakePair( QObject::trUtf8( "Typ jednotky:" ),
                             model.value( 0, "typjed_nazev" ) ) );
  content.append( qMakePair( QObject::trUtf8( "Způsob využití:" ),
                             model.value( 0, "zpvyje_nazev" ) ) );

  content.append( qMakePair( QObject::trUtf8( "Podíl jednotky na společných částech domu:" ),
                             QString( "%1/%2" )
                             .arg( model.value( 0, "jed_podil_citatel" ) )
                             .arg( model.value( 0, "jed_podil_jmenovatel" ) ) ) );

  content.append( qMakePair( QObject::trUtf8( "Katastrální území:" ),
                             makeKatastrUzemi( &model, 0 ) ) );

  mDocument->heading1( QObject::trUtf8( "Informace o jednotce" ) );
  mDocument->keyValueTable( content );

  QStringList opsubIds;
  partTelesoVlastnici( telesoModel.value( 0, "tel_id" ), opsubIds, false );

  partNemovitostOchrana( id, VfkTableModel::NJednotka );


  QStringList jedIds = QStringList() << id;
  partTelesoB1( QStringList(), QStringList(), jedIds, QStringList(), false );
  partTelesoC( QStringList(), QStringList(), jedIds, QStringList(), false );
  partTelesoD( QStringList(), QStringList(), jedIds, QStringList(), false );

}

void DocumentBuilder::pageOpravnenySubjekt( QString id )
{
  VfkTableModel opsubModel( mConnectionName );
  bool ok = opsubModel.opravnenySubjekt( id, true );
  if ( !ok )
  {
    return; //FIXME
  }

  KeyValList content;
  QString name = makeJmeno( &opsubModel, 0 );

  if ( opsubModel.value( 0, "opsub_opsub_type" ) == "OPO" )
  {
    content.append( qMakePair( QObject::trUtf8( "Název:" ), name ) );
    content.append( qMakePair( QObject::trUtf8( "Adresa:" ), makeAdresa( &opsubModel, 0 ) ) );
    content.append( qMakePair( QObject::trUtf8( "IČO:" ), makeIdentifikator( &opsubModel, 0 ) ) );
  }
  else if ( opsubModel.value( 0, "opsub_opsub_type" ) == "OFO" )
  {
    content.append( qMakePair( QObject::trUtf8( "Jméno:" ), name ) );
    content.append( qMakePair( QObject::trUtf8( "Adresa:" ), makeAdresa( &opsubModel, 0 ) ) );
    content.append( qMakePair( QObject::trUtf8( "Rodné číslo:" ), makeIdentifikator( &opsubModel, 0 ) ) );
  }
  else
  {
    content.append( qMakePair( QObject::trUtf8( "Jméno:" ), name ) );
    for ( int i = 1; i <= 2; i++ )
    {
      QString manzelId = opsubModel.value( 0, QString( "opsub_id_je_%1_partner_bsm" ).arg( i ) );
      QString desc = makeShortDescription( manzelId , VfkTableModel::OPOsoba );
      content.append( qMakePair( QString(), desc ) );
    }
  }

  content.append( qMakePair( QObject::trUtf8( "Typ:" ), opsubModel.value( 0, "charos_nazev" ) ) );



  VfkTableModel nemovitostiModel( mConnectionName );
  nemovitostiModel.vlastnikNemovitosti( id );
  QStringList telesaDesc, nemovitostDesc;
//  QStringList idColumns = QStringList() << "par_id" << "bud_id" << "jed_id";
  for ( int i = 0; i < nemovitostiModel.rowCount(); i++ )
  {
    telesaDesc << makeLVCislo( &nemovitostiModel, i );

//    foreach( QString column, idColumns )
//    {
//      nemovitostDesc << makeLongDescription( nemovitostiModel.value( i, column ),
//                                             VfkTableModel::tableName2OpravnenyPovinny( column ) );
//    }

  }
  telesaDesc.removeDuplicates();
//  nemovitostDesc.removeDuplicates();

  content.append( qMakePair( QObject::trUtf8( "Listy vlastnictví:" ),
                             telesaDesc.join( ", " ) ) );

  mDocument->heading1( QObject::trUtf8("Informace o oprávněné osobě" ) );
  mDocument->keyValueTable( content );

  partVlastnikNemovitosti( id );
  QStringList opsubIds = QStringList() << id;
  partTelesoB1( QStringList(), QStringList(), QStringList(), opsubIds, false );
  partTelesoC( QStringList(), QStringList(), QStringList(), opsubIds, false );
  partTelesoD( QStringList(), QStringList(), QStringList(), opsubIds, false );
}

void DocumentBuilder::pageSeznamParcel( QStringList ids )
{
  mDocument->heading2( QObject::trUtf8( "Seznam parcel" ) );
  mDocument->beginItemize();

  mCurrentPageParIds = ids;

  foreach( QString id, ids )
  {
    VfkTableModel model( mConnectionName );
    bool ok = model.parcela( id, false );
    if ( !ok )
    {
      continue;
    }
    mDocument->item( makeLongDescription( id, VfkTableModel::OPParcela ) );
  }
  mDocument->endItemize();
}

void DocumentBuilder::pageSeznamOsob( QStringList ids )
{
  mDocument->heading2( QObject::trUtf8( "Seznam osob" ) );
  mDocument->beginItemize();

  foreach( QString id, ids )
  {
    VfkTableModel model( mConnectionName );
    bool ok = model.opravnenySubjekt( id, true );
    if ( !ok )
    {
      continue;
    }

    mDocument->item( makeLongDescription( id, VfkTableModel::OPOsoba ) );
  }
  mDocument->endItemize();
}

void DocumentBuilder::pageSeznamBudov( QStringList ids )
{
  mDocument->heading2( QObject::trUtf8( "Seznam budov" ) );
  mDocument->beginItemize();
  mCurrentPageBudIds = ids;

  foreach( QString id, ids )
  {
    mDocument->item(  makeLongDescription( id, VfkTableModel::OPBudova ) );
  }
  mDocument->endItemize();
}

void DocumentBuilder::pageSeznamJednotek(QStringList ids)
{
  mDocument->heading2( QObject::trUtf8( "Seznam jednotek" ) );
  mDocument->beginItemize();

  foreach( QString id, ids )
  {
    mDocument->item(  makeLongDescription( id, VfkTableModel::OPJednotka ) );
  }
  mDocument->endItemize();
}

//void HtmlBuilder::pageSeznamOsob( QString name )
//{
//  VfkTableModel model( mConnectionName );
//  name.replace( QRegExp( "\\*" ), "%" );

//  bool ok = model.searchOpsub( name );
//  if ( !ok )
//  {
//    return; //FIXME
//  }
//  QStringList ids;
//  for ( int i = 0; i < model.rowCount(); i++ )
//  {
//    ids << model.value( i, "opsub_id" );
//  }

//  pageSeznamOsob( ids );
//}

void DocumentBuilder::pageSearchVlastnici( QString jmeno, QString identifikator,
                                       bool sjm, bool opo, bool ofo, QString lv )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.searchOpsub( jmeno, identifikator, sjm, opo, ofo, lv );
  if ( !ok )
  {
    return; //FIXME
  }
  QStringList ids;
  for ( int i = 0; i < model.rowCount(); i++ )
  {
    ids << model.value( i, "opsub_id" );
  }

  pageSeznamOsob( ids );
}

void DocumentBuilder::pageSearchParcely( QString parcelniCislo, QString typIndex, QString druhKod, QString lv )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.searchPar( parcelniCislo, typIndex, druhKod, lv );
  if ( !ok )
  {
    return; //FIXME
  }
  QStringList ids;
  for ( int i = 0; i < model.rowCount(); i++ )
  {
    ids << model.value( i, "par_id" );
  }

  pageSeznamParcel( ids );
}

void DocumentBuilder::pageSearchBudovy( QString domovniCislo, QString naParcele, QString zpusobVyuziti, QString lv )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.searchBud( domovniCislo, naParcele, zpusobVyuziti, lv );
  if ( !ok )
  {
    return; //FIXME
  }
  QStringList ids;
  for ( int i = 0; i < model.rowCount(); i++ )
  {
    ids << model.value( i, "bud_id" );
  }

  pageSeznamBudov( ids );
}

void DocumentBuilder::pageSearchJednotky(QString cisloJednotky, QString domovniCislo, QString naParcele, QString zpusobVyuziti, QString lv)
{
  VfkTableModel model( mConnectionName );
  bool ok = model.searchJed( cisloJednotky, domovniCislo, naParcele, zpusobVyuziti, lv );
  if ( !ok )
  {
    return; //FIXME
  }
  QStringList ids;
  for ( int i = 0; i < model.rowCount(); i++ )
  {
    ids << model.value( i, "jed_id" );
  }

  pageSeznamJednotek( ids );
}

void DocumentBuilder::pageHelp()
{
  mDocument->heading1( QObject::trUtf8( "VFK plugin" ) );
  mDocument->paragraph( QObject::trUtf8( "VFK plugin slouží pro usnadnění práce "
                                         "s českými katastrálními daty ve formátu VFK." ) );
  mDocument->heading2( QObject::trUtf8( "Kde začít?" ) );
  QString link = mDocument->link( "switchPanel?panel=import", QObject::trUtf8( "Importujte" ) );
  QString text = QObject::trUtf8( "%1 data ve formátu VFK. Během importu se vytváří databáze, "
                                  "tato operace může chvíli trvat. " ).arg( link );
  text += QObject::trUtf8( "Následně lze vyhledávat:" );
  mDocument->paragraph( text );

  mDocument->beginItemize();
  link = mDocument->link( "switchPanel?panel=search&type=0", QObject::trUtf8( "oprávněné osoby" ) );
  mDocument->item( link );
  link = mDocument->link( "switchPanel?panel=search&type=1", QObject::trUtf8( "parcely" ) );
  mDocument->item( link );
  link = mDocument->link( "switchPanel?panel=search&type=2", QObject::trUtf8( "budovy" ) );
  mDocument->item( link );
  link = mDocument->link( "switchPanel?panel=search&type=3", QObject::trUtf8( "jednotky" ) );
  mDocument->item( link );
  mDocument->endItemize();

  text = QObject::trUtf8( "Vyhledávat lze na základě různých kritérií. "
                           "Není-li kritérium zadáno, vyhledány jsou všechny nemovitosti či osoby obsažené v databázi. "
                           "Výsledky hledání jsou pak vždy zobrazeny v tomto okně.");
  mDocument->paragraph( text );

  text = QObject::trUtf8( "Výsledky hledání obsahují odkazy na další informace, "
                           "kliknutím na odkaz si tyto informace zobrazíte, "
                           "stejně jako je tomu u webového prohlížeče. "
                           "Pro procházení historie použijte tlačítka Zpět a Vpřed v panelu nástrojů nad tímto oknem.");
  mDocument->paragraph( text );

}

QString DocumentBuilder::makeShortDescription( QString id, VfkTableModel::OpravnenyPovinny nemovitost)
{
  QString text;

  if ( id.isEmpty() )
  {
    return text;
  }
  VfkTableModel model( mConnectionName );
  switch ( nemovitost )
  {
  case VfkTableModel::OPParcela:
    model.parcela( id, false );
    text = QObject::trUtf8( "Parcela: " );
    text += makeParcelniCislo( &model, 0 );
    break;

  case VfkTableModel::OPBudova:
    model.budova( id, false );
    text = QObject::trUtf8( "Budova: %1" ).arg( makeDomovniCislo( &model, 0 ) );
    break;

  case VfkTableModel::OPJednotka:
    model.jednotka( id, false );
    text = QObject::trUtf8( "Jednotka: %1" ).arg( makeJednotka( &model, 0 ) );
    break;

  case VfkTableModel::OPOsoba:
    model.opravnenySubjekt( id, true );
    if ( model.value( 0, "opsub_opsub_type" ) == "BSM" )
    {
      text = QString( "%1" ).arg( makeJmeno( &model, 0 ) );
    }
    else
    {
      text = QObject::trUtf8( "%1, %2, RČ/IČO: %3" )
          .arg( makeJmeno( &model, 0 ) )
          .arg( makeAdresa( &model, 0 ) )
          .arg( makeIdentifikator( &model, 0 ) );
    }
    break;

   default:
    break;
  }

  return text;
}
QString DocumentBuilder::makeLongDescription( QString id, VfkTableModel::OpravnenyPovinny nemovitost)
{
  QString text;

  if ( id.isEmpty() )
  {
    return text;
  }
  VfkTableModel model( mConnectionName );
  switch ( nemovitost )
  {
  case VfkTableModel::OPParcela:
    model.parcela( id, false );
    text = QObject::trUtf8( "Parcela " );
    text += makeParcelniCislo( &model, 0 );
    text += QString( ", LV %1" ).arg( makeLVCislo( &model, 0 ) );
    break;

  case VfkTableModel::OPBudova:
    model.budova( id, false );
    text = QObject::trUtf8( "Budova %1" )
        .arg( makeDomovniCislo( &model, 0 ) );

    text += QObject::trUtf8( " na parcele %1" )
        .arg( makeParcelniCislo( &model, 0 ) );

    text += QString( ", LV %1" ).arg( makeLVCislo( &model, 0 ) );
    break;

  case VfkTableModel::OPJednotka:
    model.jednotka( id, true );
    text = QObject::trUtf8( "Jednotka %1" )
        .arg( makeJednotka( &model, 0 ) );

    text +=  QObject::trUtf8( " v budově  %1" )
        .arg( makeDomovniCislo( &model, 0 ) );

    text += QObject::trUtf8( " na parcele %1" )
        .arg( makeParcelniCislo( &model, 0 ) );

    text += QString( ", LV %1" ).arg( makeLVCislo( &model, 0 ) );
    break;

  case VfkTableModel::OPOsoba:
    text += makeShortDescription( id, nemovitost );
    break;

   default:
    break;
  }

  return text;
}

QString DocumentBuilder::makeAdresa( const VfkTableModel* model, int row )
{
  QString cislo_domovni = model->value( row, "opsub_cislo_domovni" );
  QString cislo_orientacni = model->value( row, "opsub_cislo_orientacni" );
  QString ulice = model->value( row, "opsub_nazev_ulice" );
  QString cast_obce = model->value( row, "opsub_cast_obce" );
  QString obec = model->value( row, "opsub_obec" );
  QString mestska_cast = model->value( row, "opsub_mestska_cast" );
  QString psc = model->value( row, "opsub_psc" );

  QString cislo = cislo_domovni;
  if ( !cislo_orientacni.isEmpty() )
  {
    cislo += "/" + cislo_orientacni;
  }
  QStringList adresa;
  if ( ulice.isEmpty() )
  {
    adresa << obec;
    cislo.isEmpty() ? adresa : adresa << cislo;
    cast_obce.isEmpty() ? adresa : adresa << cast_obce;
  }
  else
  {
    adresa << ulice;
    cislo.isEmpty() ? adresa : adresa << cislo;
    cast_obce.isEmpty() ? (mestska_cast.isEmpty() ? adresa : adresa << mestska_cast) : adresa << cast_obce;
    adresa << obec;
  }
  psc.isEmpty() ? adresa : adresa << psc;

  return adresa.join( ", " );
}

QString DocumentBuilder::makeJmeno( const VfkTableModel *model, int row )
{
  QString jmeno;

  if ( model->value( row, "opsub_opsub_type" ) == "OFO")
  {
    jmeno += model->value( row, "opsub_titul_pred_jmenem" ) + " ";
    jmeno += model->value( row, "opsub_prijmeni" ) + " ";
    jmeno += model->value( row, "opsub_jmeno" ) + " ";
    jmeno += model->value( row, "opsub_titul_za_jmenem" );
  }
  else
  {
    jmeno += model->value( row, "opsub_nazev" );
  }

  return mDocument->link( QString( "showText?page=opsub&id=%1" )
                          .arg( model->value( row, "opsub_id" ) ), jmeno );
}

QString DocumentBuilder::makeIdentifikator(const VfkTableModel *model, int row)
{
  QString type = model->value( row, "opsub_opsub_type" );
  QString identifikator;
  if ( type == "OPO" )
  {
    identifikator = model->value( row, "opsub_ico" );
  }
  else if ( type == "OFO" )
  {
    identifikator = model->value( row, "opsub_rodne_cislo" );
  }

  return identifikator;
}

QString DocumentBuilder::makeParcelniCislo( const VfkTableModel *model, int row )
{
  QString cislo;

  cislo += model->value( row, "par_kmenove_cislo_par" );
  QString poddeleni = model->value( row, "par_poddeleni_cisla_par" );
  if ( !poddeleni.isEmpty() )
  {
    cislo += "/" + poddeleni;
  }
  QString st;
  if (model->value( row, "drupoz_stavebni_parcela" ).isEmpty())
  {
    qWarning() << "neni drupoz_stavebni_parcela";
  }
  if ( mDveRadyCislovani && Domains::anoNe( model->value( row, "drupoz_stavebni_parcela" ) ) )
  {
    st = "st.";
  }
  QString link = mDocument->link( QString( "showText?page=par&id=%1" )
                                  .arg( model->value( row, "par_id" ) ), cislo );
  return QString( "%1 %2" ).arg( st ).arg( link );
}

QString DocumentBuilder::makeDomovniCislo( const VfkTableModel *model, int row )
{
  return mDocument->link( QString( "showText?page=bud&id=%1" ).arg( model->value( row, "bud_id" ) ),
                          QString( "%1 %2" )
                          .arg( model->value( row, "typbud_zkratka" ) )
                          .arg( model->value( row, "bud_cislo_domovni" ) ) );
}

QString DocumentBuilder::makeJednotka( const VfkTableModel *model, int row )
{
  return mDocument->link( QString( "showText?page=jed&id=%1" ).arg( model->value( row, "jed_id" ) ),
                          QString( "%1/%2" )
                          .arg( model->value( row, "bud_cislo_domovni" ) )
                          .arg( model->value( row, "jed_cislo_jednotky" ) ) );
}

QString DocumentBuilder::makeListina( const VfkTableModel *model, int row )
{
  return QObject::trUtf8( "Listina: %1 %2" )
      .arg( model->value( row, "typlis_nazev" ) )
      .arg( model->value( row, "dul_nazev" ) );
}

QString DocumentBuilder::makeLVCislo(const VfkTableModel *model, int row)
{
  return mDocument->link( QString( "showText?page=tel&id=%1" ).arg( model->value( row, "tel_id" ) ),
                          model->value( row, "tel_cislo_tel" ) );
}

QString DocumentBuilder::makeKatastrUzemi(const VfkTableModel *model, int row)
{
  return QString( "%1 %2" )
      .arg( model->value( row, "katuze_nazev" ) )
      .arg( model->value( row, "par_katuze_kod" ) );
}

QString DocumentBuilder::makeCastObce(const VfkTableModel *model, int row)
{
  return QString( "%1 %2" )
      .arg( model->value( row, "casobc_nazev" ) )
      .arg( model->value( row, "casobc_kod" ) );
}

QString DocumentBuilder::makeObec(const VfkTableModel *model, int row)
{
  return QString( "%1 %2" )
      .arg( model->value( row, "obce_nazev" ) )
      .arg( model->value( row, "obce_kod" ) );
}

void DocumentBuilder::saveDefinitionPoint(QString id, VfkTableModel::Nemovitost nemovitost )
{
  VfkTableModel model( mConnectionName );
  bool ok = model.definicniBod( id, nemovitost );
  if ( !ok )
  {
    return;
  }
  mCurrentDefinitionPoint.first = model.value( 0, "obdebo_souradnice_x" );
  mCurrentDefinitionPoint.second = model.value( 0, "obdebo_souradnice_y" );
}

