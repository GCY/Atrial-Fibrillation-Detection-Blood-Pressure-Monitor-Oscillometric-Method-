#include "pidargsdlg.h"

IMPLEMENT_CLASS(PIDArgsDialog,wxDialog)

PIDArgsDialog::PIDArgsDialog()
{
   Init();
}

PIDArgsDialog::PIDArgsDialog(wxWindow *parent,wxWindowID id,const wxString &caption,const wxPoint &pos,const wxSize &size,long style)
{
   Create(parent,id,caption,pos,size,style);
   Init();
}

inline void PIDArgsDialog::Init()
{
}

bool PIDArgsDialog::Create(wxWindow *parent,wxWindowID id,const wxString &caption,const wxPoint &pos,const wxSize &size,long style)
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

void PIDArgsDialog::CreateControls()
{
   wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);
   this->SetSizer(top);

   wxBoxSizer *box = new wxBoxSizer(wxHORIZONTAL);
   top->Add(box,0,wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL,5);

   wxStaticText *p_text = new wxStaticText(this, wxID_ANY, "P:");   
   p_sbd = new wxSpinCtrlDouble( this);
   p_sbd->SetRange(0.0,99.9);
   p_sbd->SetValue(2.8);
   p_sbd->SetIncrement(0.1);
   box->Add(p_text,0,wxALL,5);
   box->Add(p_sbd,0,wxALL,5);

   wxStaticText *i_text = new wxStaticText(this, wxID_ANY, "I:");   
   i_sbd = new wxSpinCtrlDouble( this);
   i_sbd->SetRange(0.0,99.9);
   i_sbd->SetValue(0.4);
   i_sbd->SetIncrement(0.1);
   box->Add(i_text,0,wxALL,5);
   box->Add(i_sbd,0,wxALL,5);

   wxStaticText *d_text = new wxStaticText(this, wxID_ANY, "D:");   
   d_sbd = new wxSpinCtrlDouble( this);
   d_sbd->SetRange(0.0,99.9);
   d_sbd->SetValue(0.2);
   d_sbd->SetIncrement(0.1);
   box->Add(d_text,0,wxALL,5);
   box->Add(d_sbd,0,wxALL,5);  

   wxStaticText *sp_text = new wxStaticText(this, wxID_ANY, "mmHg/s:");   
   sp_sbd = new wxSpinCtrlDouble( this);
   sp_sbd->SetRange(1,30);
   sp_sbd->SetValue(3);
   sp_sbd->SetIncrement(1);
   box->Add(sp_text,0,wxALL,5);
   box->Add(sp_sbd,0,wxALL,5);     

   wxBoxSizer *ResetOkCancelBox = new wxBoxSizer(wxHORIZONTAL);
   top->Add(ResetOkCancelBox,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

   wxButton *ok = new wxButton(this,wxID_OK,wxT("&Ok"),wxDefaultPosition,wxDefaultSize,0);
   ResetOkCancelBox->Add(ok,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

   wxButton *cancel = new wxButton(this,wxID_CANCEL,wxT("&Cancel"),wxDefaultPosition,wxDefaultSize,0);
   ResetOkCancelBox->Add(cancel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);   

}

bool PIDArgsDialog::TransferDataToWindow()
{
   return true;
}

bool PIDArgsDialog::TransferDataFromWindow()
{
   return true;
}
