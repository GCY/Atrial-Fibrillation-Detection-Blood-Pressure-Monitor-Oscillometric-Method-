#ifndef __BP_ALGO_DIALOG__
#define __BP_ALGO_DIALOG__
#include <wx/wx.h>
#include <wx/spinctrl.h>

class BPAlgoDialog:public wxDialog
{
   public:
      BPAlgoDialog();

      BPAlgoDialog(wxWindow *parent,
	    wxWindowID id = wxID_ANY,
	    const wxString &caption = wxT("BP Algorithm As/Ad Tuning"),
	    const wxPoint &pos = wxDefaultPosition,
	    const wxSize &size = wxDefaultSize,
	    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

      bool Create(wxWindow *parent,
	    wxWindowID id = wxID_ANY,
	    const wxString &caption = wxT("BP Algorithm As/Ad Tuning"),
	    const wxPoint &pos = wxDefaultPosition,
	    const wxSize &size = wxDefaultSize,
	    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

      void Init();
      void CreateControls();

      bool TransferDataToWindow();
      bool TransferDataFromWindow();

      double GetAs(){return as_sbd->GetValue();}
      double GetAd(){return ad_sbd->GetValue();}

   private:
/*
      EnumSerial enumserial;

      wxChoice *device_path;
      wxChoice *baud_rate;
*/
      wxSpinCtrlDouble  *as_sbd;
      wxSpinCtrlDouble  *ad_sbd;
      DECLARE_CLASS(BPAlgoDialog)
};

#endif
