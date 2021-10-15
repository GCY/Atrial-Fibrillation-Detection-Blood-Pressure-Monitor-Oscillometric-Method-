#include "bpalgodlg.h"

IMPLEMENT_CLASS(BPAlgoDialog,wxDialog)

BPAlgoDialog::BPAlgoDialog()
{
   Init();
}

BPAlgoDialog::BPAlgoDialog(wxWindow *parent,wxWindowID id,const wxString &caption,const wxPoint &pos,const wxSize &size,long style)
{
   Create(parent,id,caption,pos,size,style);
   Init();
}

inline void BPAlgoDialog::Init()
{
}

bool BPAlgoDialog::Create(wxWindow *parent,wxWindowID id,const wxString &caption,const wxPoint &pos,const wxSize &size,long style)
{
   SetExtraStyle(wxWS_EX_BLOCK_EVENTS | wxDIALOG_EX_CONTEXTHELP);

   if(!wxDialog::Create(parent,id,caption,pos,size,style)){
      return false;
   }

   CreateControls();

   GetSizer()->Fit(this);
   GetSizer()->SetSizeHints(this);

   Center();

   return true;
}

void BPAlgoDialog::CreateControls()
{
   wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);
   this->SetSizer(top);

   wxBoxSizer *box = new wxBoxSizer(wxHORIZONTAL);
   top->Add(box,0,wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);

   wxStaticText *as_text = new wxStaticText(this, wxID_ANY, "As(%):");   
   as_sbd = new wxSpinCtrlDouble( this);
   as_sbd->SetRange(1,99);
   as_sbd->SetValue(65);
   as_sbd->SetIncrement(1);
   box->Add(as_text,0,wxALL,5);
   box->Add(as_sbd,0,wxALL,5);

   wxStaticText *ad_text = new wxStaticText(this, wxID_ANY, "Ad(%):");   
   ad_sbd = new wxSpinCtrlDouble( this);
   ad_sbd->SetRange(1,99);
   ad_sbd->SetValue(70);
   ad_sbd->SetIncrement(1);
   box->Add(ad_text,0,wxALL,5);
   box->Add(ad_sbd,0,wxALL,5);

   wxBoxSizer *ResetOkCancelBox = new wxBoxSizer(wxHORIZONTAL);
   top->Add(ResetOkCancelBox,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

   wxButton *ok = new wxButton(this,wxID_OK,wxT("&Ok"),wxDefaultPosition,wxDefaultSize,0);
   ResetOkCancelBox->Add(ok,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

   wxButton *cancel = new wxButton(this,wxID_CANCEL,wxT("&Cancel"),wxDefaultPosition,wxDefaultSize,0);
   ResetOkCancelBox->Add(cancel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);   

}

bool BPAlgoDialog::TransferDataToWindow()
{
   return true;
}

bool BPAlgoDialog::TransferDataFromWindow()
{
   return true;
}
