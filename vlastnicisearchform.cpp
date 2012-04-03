#include "vlastnicisearchform.h"

VlastniciSearchForm::VlastniciSearchForm(QWidget *parent) :
  QWidget(parent)
{
  setupUi(this);

  connect( ofoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSetRcIcoEnabled() ) );
  connect( opoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSetRcIcoEnabled() ) );

  connect( jmenoLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( rcIcoLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( ofoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( opoCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( sjmCheckBox, SIGNAL( clicked() ), this, SLOT( vlastniciSearchEnabled() ) );
  connect( lvVlastniciLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( vlastniciSearchEnabled() ) );
}

void VlastniciSearchForm::postInit()
{
  vlastniciSearchEnabled();
}

void VlastniciSearchForm::vlastniciSearchEnabled()
{
  if ( jmenoLineEdit->text().isEmpty() && rcIcoLineEdit->text().isEmpty() && lvVlastniciLineEdit->text().isEmpty() )
  {
    emit searchEnabled( false );
  }
  else
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
