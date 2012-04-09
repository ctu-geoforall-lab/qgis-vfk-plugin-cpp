#include "vlastnicisearchform.h"

VlastniciSearchForm::VlastniciSearchForm(QWidget *parent) :
  QWidget(parent)
{
  setupUi(this);

  connect( ofoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSetRcIcoEnabled() ) );
  connect( opoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSetRcIcoEnabled() ) );

  connect( ofoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( opoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( sjmCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSearchEnabled() ) );
}

void VlastniciSearchForm::vlastniciSearchEnabled()
{
  if ( !( ofoCheckBox->isChecked() || opoCheckBox->isChecked() || sjmCheckBox->isChecked() ) )
  {
    emit searchEnabled( false );
  }
  else
  {
    emit searchEnabled( true );
  }
}

void VlastniciSearchForm::vlastniciSetRcIcoEnabled()
{
  if ( ofoCheckBox->isChecked() || opoCheckBox->isChecked() )
  {
    rcIcoLineEdit->setEnabled(true);
  }
  else
  {
    rcIcoLineEdit->setEnabled(false);
  }
}
