#ifndef __PID_ARGS_DIALOG__
#define __PID_ARGS_DIALOG__
#include <wx/wx.h>
#include <wx/spinctrl.h>

class PIDArgsDialog:public wxDialog
{
   public:
      PIDArgsDialog();

      PIDArgsDialog(wxWindow *parent,
	    wxWindowID id = wxID_ANY,
	    const wxString &caption = wxT("PID Tuning"),
	    const wxPoint &pos = wxDefaultPosition,
	    const wxSize &size = wxDefaultSize,
	    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

      bool Create(wxWindow *parent,
	    wxWindowID id = wxID_ANY,
	    const wxString &caption = wxT("PID Tuning"),
	    const wxPoint &pos = wxDefaultPosition,
	    const wxSize &size = wxDefaultSize,
	    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

      void Init();
      void CreateControls();

      bool TransferDataToWindow();
      bool TransferDataFromWindow();

      double GetP(){return p_sbd->GetValue();}
      double GetI(){return i_sbd->GetValue();}
      double GetD(){return d_sbd->GetValue();}

   private:
/*
      EnumSerial enumserial;

      wxChoice *device_path;
      wxChoice *baud_rate;
*/
      wxSpinCtrlDouble  *p_sbd;
      wxSpinCtrlDouble  *i_sbd;
      wxSpinCtrlDouble  *d_sbd;
      DECLARE_CLASS(PIDArgsDialog)
};

#endif
