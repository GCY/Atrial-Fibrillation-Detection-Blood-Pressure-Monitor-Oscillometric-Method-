#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/valnum.h>
#include <wx/thread.h>

#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime> 
#include <iostream>
#include <chrono> 
#include <iomanip>
#include <sstream>
#include <string>

#include "mathplot.h"
#include "connectargsdlg.h"
#include "serialport.h"

#include "define.h"
#include "adaptive_algorithm.h"
#include "FIR.h"
#include "polynomial_regression.h"

const unsigned int taps = 19;
/*100hz 0.7Hz~8.9Hz*/
float coeff1[taps] = {-0.043047,-0.048389,-0.042549,-0.023677,0.0073069,0.046611,0.088179,0.12489,0.15008,0.15905,0.15008,0.12489,0.088179,0.046611,0.0073069,-0.023677,-0.042549,-0.048389,-0.043047};
/* 100Hz 0.5Hz low pass*/
float coeff2[taps] = {0.050186,0.050989,0.051705,0.052331,0.052864,0.053303,0.053646,0.053892,0.05404,0.054089,0.05404,0.053892,0.053646,0.053303,0.052864,0.052331,0.051705,0.050989,0.050186};

float buffer1[taps];
unsigned offset1;
float buffer2[taps];
unsigned offset2;

enum{
   ID_EXIT = 200,
   ID_OPEN,
   ID_RECORD,
   ID_VCP,
   ID_CLEAR_ALL_PLOT,
   ID_CONNECT_DEVICE,
   ID_MEASUREMENT,
   ID_PRESSURIZE,
   ID_LEAK,
   ID_MODE
};

enum{
   EVT_SERIAL_PORT_READ = 625,
   EVT_REFRESH_PLOT
};

class Thread;

class App:public wxApp
{
   public:
      bool OnInit();
};

class Frame:public wxFrame
{
   public:
      Frame(const wxString&);
      ~Frame();

      void CreateUI();

      void OnProcess(wxCommandEvent&);

      void OnClearAllPlot(wxCommandEvent&);
      void OnOpen(wxCommandEvent&);
      void OnKeyDown(wxKeyEvent& event);
      void OnConnectDevice(wxCommandEvent&);
      void OnMeasurment(wxCommandEvent&);
      void OnPressurize(wxCommandEvent&);
      void OnLeak(wxCommandEvent&);
      void OnMode(wxCommandEvent&);
      void OnExit(wxCommandEvent&);

      void ReadCurve();
      void BPCalculator();

      void ClearAllData();

      void OnFit(wxCommandEvent&);

      void OnThreadEvent(wxThreadEvent &event);

   private:

      void split(const std::string& s, std::vector<std::string>& sv, const char delim = ' '){
	 sv.clear();
	 std::istringstream iss(s);
	 std::string temp;

	 while (std::getline(iss, temp, delim)){
	    sv.emplace_back(std::move(temp));
	 }
      }      

      wxMenu *vcp;
      wxThread *thread;

      SerialPort serial;     
      bool is_open;

      bool run_flag;

      mpWindow *dc_plot;
      mpWindow *ac_plot;  

      mpFXYVector* dc_layer;
      std::vector<double> dc_layer_x, dc_layer_y;

      mpFXYVector* ac_layer;
      std::vector<double> ac_layer_x, ac_layer_y;
      mpFXYVector* peak_layer;
      std::vector<double> peak_layer_x, peak_layer_y;
      mpFXYVector* trough_layer;
      std::vector<double> trough_layer_x, trough_layer_y;       
      mpFXYVector* map_layer;
      std::vector<double> map_layer_x, map_layer_y;       
      mpFXYVector* sbp_layer;
      std::vector<double> sbp_layer_x, sbp_layer_y;       
      mpFXYVector* dbp_layer;
      std::vector<double> dbp_layer_x, dbp_layer_y;             
      static const wxString peak_str;
      static const wxString trough_str;
      static const wxString map_str;   
      static const wxString sbp_str;
      static const wxString dbp_str;      

      std::chrono::steady_clock::time_point start_time;
      int time_to_index;      

      std::vector<std::string> serial_pool;

      uint32_t sampling_rate;
      std::chrono::steady_clock::time_point sampling_time;

      wxTextCtrl *ad_am_text;
      wxTextCtrl *as_am_text;

      FIRInfo info1;
      FIRInfo info2;

      double baseline;
      uint32_t init_baseline_count;

      bool mode_switch;

      wxButton *measurement_button;
      wxButton *mode_button;
      wxButton *pressurize_button;
      wxButton *leak_button;

      std::vector<double> a;

      bool first_point_flag;

      uint32_t folder_counter;

      double pressure;
      double dc;
      double MAP;
      double SBP;
      double DBP;

      static const uint32_t PRESSURE_MIN = 50;
      static const uint32_t PRESSURE_MAX = 160;

      static const uint32_t MEASURMENT_TIME = 35;
      std::chrono::steady_clock::time_point measurment_time;
      double dc_array[MEASURMENT_TIME*2];
      double ac_array[MEASURMENT_TIME*2];
      uint32_t index_array[MEASURMENT_TIME*2];
      uint32_t pulse_index;

      static const uint32_t calibration_point = SAMPLING_RATE * 2;

      std::chrono::steady_clock::time_point input_time;

      std::fstream record_file;
      std::fstream map_file;
      std::fstream ac_peak_file;

      DECLARE_EVENT_TABLE();
};

const wxString Frame::peak_str = wxT("Peak");
const wxString Frame::trough_str = wxT("Trough");
const wxString Frame::map_str = wxT("MAP");
const wxString Frame::sbp_str = wxT("SBP");
const wxString Frame::dbp_str = wxT("DBP");

class Thread:public wxThread
{
   public:
      Thread(Frame*,wxEvtHandler*);

      void* Entry();

   private:
      std::chrono::steady_clock::time_point refresh_last;
      std::chrono::steady_clock::time_point read_last;
      Frame *frame;
      wxEvtHandler *handler;
};

   IMPLEMENT_APP(App)
DECLARE_APP(App)

   BEGIN_EVENT_TABLE(Frame,wxFrame)
   EVT_MENU(ID_EXIT,Frame::OnExit)
   EVT_MENU(ID_OPEN,Frame::OnOpen)
   EVT_MENU(ID_CLEAR_ALL_PLOT,Frame::OnClearAllPlot)
   EVT_MENU(ID_CONNECT_DEVICE,Frame::OnConnectDevice)
   EVT_BUTTON(ID_MEASUREMENT,Frame::OnMeasurment)
   EVT_BUTTON(ID_PRESSURIZE,Frame::OnPressurize)
   EVT_BUTTON(ID_LEAK,Frame::OnLeak)
   EVT_BUTTON(ID_MODE,Frame::OnMode)
   EVT_MENU(mpID_FIT, Frame::OnFit)
   EVT_THREAD(wxID_ANY, Frame::OnThreadEvent)
END_EVENT_TABLE()

bool App::OnInit()
{
   Frame *frame = new Frame(wxT("Oscillometric Blood Pressure Monitor"));

   frame->Show(true);

   return true;
}

Frame::Frame(const wxString &title):wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(1050,700),wxMINIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU)
{
   run_flag = true;

   mode_switch = false;

   info1.taps = taps;
   info1.coeff = coeff1;
   info1.buffer = buffer1;
   info1.offset = offset1;

   info2.taps = taps;
   info2.coeff = coeff2;
   info2.buffer = buffer2;
   info2.offset = offset2;   

   CreateUI();

   ReadCurve();

   Bind(wxEVT_CHAR_HOOK,&Frame::OnKeyDown,this);

   dc_layer = new mpFXYVector(wxT("DC Signal"),mpALIGN_NE);
   dc_layer->ShowName(false);
   dc_layer->SetContinuity(true);
   wxPen vectorpen(*wxRED, 2, wxSOLID);
   dc_layer->SetPen(vectorpen);
   dc_layer->SetDrawOutsideMargins(false);

   wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
   mpScaleX* xaxis = new mpScaleX(wxT("Time（index）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   mpScaleY* yaxis = new mpScaleY(wxT("mmHg"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   dc_plot->SetMargins(30, 30, 50, 100);
   dc_plot->AddLayer(     xaxis );
   dc_plot->AddLayer(     yaxis );
   dc_plot->AddLayer(     dc_layer );
   wxBrush hatch(wxColour(200,200,200), wxSOLID);

   mpInfoLegend* leg;
   dc_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);

   dc_plot->EnableDoubleBuffer(true);
   dc_plot->SetMPScrollbars(false);
   dc_plot->Fit();

   ac_layer = new mpFXYVector(wxT("AC Signal"),mpALIGN_NE);
   ac_layer->ShowName(false);
   ac_layer->SetContinuity(true);
   wxPen vectorpen1(*wxCYAN, 2, wxSOLID);
   ac_layer->SetPen(vectorpen1);
   ac_layer->SetDrawOutsideMargins(false);

   xaxis = new mpScaleX(wxT("Time（index）"), mpALIGN_BOTTOM, true, mpX_NORMAL);
   yaxis = new mpScaleY(wxT("Amplitude（12bit）"), mpALIGN_LEFT, true);
   xaxis->SetFont(graphFont);
   yaxis->SetFont(graphFont);
   xaxis->SetDrawOutsideMargins(false);
   yaxis->SetDrawOutsideMargins(false);
   ac_plot->SetMargins(30, 30, 50, 100);
   ac_plot->AddLayer(     xaxis );
   ac_plot->AddLayer(     yaxis );
   ac_plot->AddLayer(     ac_layer );

   ac_plot->AddLayer( leg = new mpInfoLegend(wxRect(200,20,40,40), wxTRANSPARENT_BRUSH));
   leg->SetVisible(true);

   ac_plot->EnableDoubleBuffer(true);
   ac_plot->SetMPScrollbars(false);
   ac_plot->Fit();

   peak_layer = new mpFXYVector(peak_str,mpALIGN_NE);
   peak_layer->ShowName(false);
   peak_layer->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen1_2(*wxRED, 7, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_2(*wxRED, 7, wxUSER_DASH);
#endif
   peak_layer->SetPen(vectorpen1_2);
   peak_layer->SetDrawOutsideMargins(false);   
   ac_plot->AddLayer(     peak_layer );

   trough_layer = new mpFXYVector(trough_str,mpALIGN_NE);
   trough_layer->ShowName(false);
   trough_layer->SetContinuity(false);
#ifdef _WIN_ 
   wxPen vectorpen1_3(*wxGREEN, 7, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_3(*wxGREEN, 7, wxUSER_DASH);
#endif
   trough_layer->SetPen(vectorpen1_3);
   trough_layer->SetDrawOutsideMargins(false);   
   ac_plot->AddLayer(     trough_layer );  


   map_layer = new mpFXYVector(map_str,mpALIGN_NE);
   map_layer->ShowName(false);
   map_layer->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen1_4(*wxYELLOW, 10, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_4(*wxYELLOW, 10, wxUSER_DASH);
#endif
   map_layer->SetPen(vectorpen1_4);
   map_layer->SetDrawOutsideMargins(false);   
   ac_plot->AddLayer(     map_layer );   

   sbp_layer = new mpFXYVector(sbp_str,mpALIGN_NE);
   sbp_layer->ShowName(false);
   sbp_layer->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen1_5(*wxBLUE, 10, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_5(*wxBLUE, 10, wxUSER_DASH);
#endif
   sbp_layer->SetPen(vectorpen1_5);
   sbp_layer->SetDrawOutsideMargins(false);   
   ac_plot->AddLayer(     sbp_layer );   

   dbp_layer = new mpFXYVector(dbp_str,mpALIGN_NE);
   dbp_layer->ShowName(false);
   dbp_layer->SetContinuity(false);
#ifdef _WIN_
   wxPen vectorpen1_6(*wxLIGHT_GREY, 10, wxPENSTYLE_DOT_DASH);
#elif _MAC_
   wxPen vectorpen1_6(*wxLIGHT_GREY, 10, wxUSER_DASH);
#endif
   dbp_layer->SetPen(vectorpen1_6);
   dbp_layer->SetDrawOutsideMargins(false);   
   ac_plot->AddLayer(     dbp_layer );   


   sampling_time = std::chrono::steady_clock::now();
   sampling_rate = 0;

   start_time = std::chrono::steady_clock::now();
   time_to_index = 0;

   std::srand(std::time(nullptr));

   baseline = 0;
   init_baseline_count = 0;

   first_point_flag = false;
   folder_counter = 0;


   thread = new Thread(this,this);
   thread->Create();
   thread->Run();
}

Frame::~Frame()
{
   if(record_file.is_open()){
      record_file.close();
   }
   if(map_file.is_open()){
      map_file.close();
   }
   if(ac_peak_file.is_open()){
      ac_peak_file.close();
   }
   if(thread){
      thread->Delete();
      thread = NULL;
   }
}

void Frame::CreateUI()
{
   wxMenu *file = new wxMenu;
   file->Append(ID_OPEN,wxT("O&pen\tAlt-o"),wxT("Open"));
   file->Append(ID_RECORD,wxT("R&cord\tAlt-r"),wxT("Record"));
   file->Append(ID_EXIT,wxT("E&xit\tAlt-e"),wxT("Exit"));

   vcp = new wxMenu();
   wxMenu *tools = new wxMenu;
   tools->Append(ID_CONNECT_DEVICE,wxT("V&CP\tAlt-v"),wxT("Virtual COM Port"));


   wxMenu *view = new wxMenu;
   view->Append(mpID_FIT,wxT("F&it\tAlt-f"),wxT("Fit"));
   view->Append(ID_CLEAR_ALL_PLOT,wxT("C&lear Plot\tAlt-c"),wxT("Clear All Plot"));

   wxMenuBar *bar = new wxMenuBar;

   bar->Append(file,wxT("File"));
   bar->Append(tools,wxT("Tools"));
   bar->Append(view,wxT("View"));
   SetMenuBar(bar);

   dc_plot = new mpWindow( this, -1, wxPoint(15,10), wxSize(1020,300), wxBORDER_SUNKEN );
   dc_plot->EnableDoubleBuffer(true);
   ac_plot = new mpWindow( this, -1, wxPoint(15,330), wxSize(1024,300), wxBORDER_SUNKEN );
   ac_plot->EnableDoubleBuffer(true);

   wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);
   this->SetSizer(top);
   top->Add(dc_plot,1,wxEXPAND);
   top->Add(ac_plot,1,wxEXPAND);

   wxBoxSizer *button_box = new wxBoxSizer(wxHORIZONTAL);
   measurement_button = new wxButton(this,ID_MEASUREMENT,wxT("&Measurement"),wxDefaultPosition,wxSize(100,35));
   mode_button = new wxButton(this,ID_MODE,wxT("&USB Mode"),wxDefaultPosition,wxSize(100,35));
   pressurize_button = new wxButton(this,ID_PRESSURIZE,wxT("&Pressurize"),wxDefaultPosition,wxSize(100,35));
   pressurize_button->Enable(false);
   leak_button = new wxButton(this,ID_LEAK,wxT("&Leak"),wxDefaultPosition,wxSize(100,35));
   leak_button->Enable(false);

   double value;
   wxFloatingPointValidator<double> _val(2,&value,wxNUM_VAL_ZERO_AS_BLANK);
   wxStaticText *ad_am = new wxStaticText(this,wxID_ANY,wxT("Ad/Am:"));
   _val.SetRange(0.0100f,0.99f); 
   ad_am_text = new wxTextCtrl(this,
	 wxID_ANY, 
	 "0.7", 
	 wxDefaultPosition, 
	 wxDefaultSize,
	 wxTE_PROCESS_ENTER,
	 _val,
	 wxTextCtrlNameStr);
   wxStaticText *as_am = new wxStaticText(this,wxID_ANY,wxT("As/Am:"));
   _val.SetRange(0.0100f,0.99f); 
   as_am_text = new wxTextCtrl(this,
	 wxID_ANY, 
	 "0.65", 
	 wxDefaultPosition, 
	 wxDefaultSize,
	 wxTE_PROCESS_ENTER,
	 _val,
	 wxTextCtrlNameStr);

   button_box->Add(ad_am, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(ad_am_text, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(as_am, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(as_am_text, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(measurement_button, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(mode_button, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(pressurize_button, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);
   button_box->Add(leak_button, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 3);

   top->Add(button_box,0,wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

   CreateStatusBar(1);
   SetStatusText(wxT("Oscillometric Blood Pressure Monitor"));   
}

void Frame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
   dc_plot->Fit();
   ac_plot->Fit();
}

void Frame::OnClearAllPlot(wxCommandEvent &event)
{
   ClearAllData();
}

void Frame::ClearAllData()
{
   dc_plot->DelAllLayers(true);
   ac_plot->DelAllLayers(true);

   dc_layer_x.clear();
   dc_layer_y.clear();

}

void Frame::ReadCurve()
{
   std::vector<double> x,y;

   std::string string;
   std::fstream input("curve.csv",std::ios::in);
   if(input.is_open()){
      while(input >> string){
	 std::vector<std::string> sv;
	 split(string, sv, ',');
	 x.push_back(atoi(sv[0].c_str()));
	 y.push_back(atoi(sv[1].c_str()));
      }
      input.close();

      unsigned int base = x[0];
      for(int i = 0; i < x.size();++i){
	 x[i] -= base;
      }

      a = PolynomialRegression(x,y,2);

      std::fstream equation_file("equation.txt",std::ios::out);
      for(int i = 0;i < a.size();++i){
	 equation_file << std::fixed << std::setprecision(10) << "(" << a[i] << ")" << "x^" << i << ((i+1 == a.size())?"":"+");
      }

      equation_file.close();
   }
   else{
      wxMessageBox(wxT("Can't find curve.csv"));
      a.push_back(-1.0620516546f);
      a.push_back(0.1262915457f);
      a.push_back(-0.0000012119f);

   }
}

void Frame::OnMeasurment(wxCommandEvent &event)
{
   if(is_open){
      unsigned char measurement_str[2] = "M";
      serial.Write(measurement_str,2);

      FIR_reset_buffer(&info1);
      FIR_reset_buffer(&info2);

      baseline = 0;
      init_baseline_count = 0;
      first_point_flag = false;

      pressure = 0;
      MAP = 0;
      SBP = 0;
      DBP = 0;

      for(int i = 0;i < pulse_index;++i){
	 dc_array[i] = 0;
	 ac_array[i] = 0;
	 index_array[i] = 0;
      }
      pulse_index = 0;

      if(record_file.is_open()){
	 record_file.close();
      }
      if(map_file.is_open()){
	 map_file.close();
      }
      if(ac_peak_file.is_open()){
	 ac_peak_file.close();
      }

      dc_layer_x.clear();
      dc_layer_y.clear();
      ac_layer_x.clear();
      ac_layer_y.clear();
      peak_layer_x.clear();
      peak_layer_y.clear();
      trough_layer_x.clear();
      trough_layer_y.clear();
      map_layer_x.clear();
      map_layer_y.clear();
      sbp_layer_x.clear();
      sbp_layer_y.clear();
      dbp_layer_x.clear();
      dbp_layer_y.clear();      

      measurment_time = std::chrono::steady_clock::now();

   }
   event.Skip();
}

void Frame::OnPressurize(wxCommandEvent &event)
{
   if(is_open){
      unsigned char pressurize_str[2] = "P";
      serial.Write(pressurize_str,2);
   }   
   event.Skip();
}

void Frame::OnLeak(wxCommandEvent &event)
{
   if(is_open){
      unsigned char leak_str[2] = "L";
      serial.Write(leak_str,2);
   }   
   event.Skip();
}

void Frame::OnMode(wxCommandEvent &event)
{
   if(is_open){
      unsigned char mode_str[2] = "S";
      serial.Write(mode_str,2);

      mode_switch ^= true;
      if(mode_switch){
	 measurement_button->Enable(false);
	 pressurize_button->Enable(true);
	 leak_button->Enable(true);
	 mode_button->SetLabel(wxT("&Calib Mode"));
      }
      else{
	 measurement_button->Enable(true);
	 pressurize_button->Enable(false);
	 leak_button->Enable(false);       
	 mode_button->SetLabel(wxT("&USB Mode"));
      }
   }
   event.Skip();
}


void Frame::OnOpen(wxCommandEvent &event)
{
   wxFileDialog 
      ofd(this, wxT("Open RAW Data file"), "", "",
	    "RAW Data files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
   if(ofd.ShowModal() == wxID_CANCEL){
      return;     // the user changed idea...
   }
   //LoadFile(ofd.GetPath());
}

void Frame::OnConnectDevice(wxCommandEvent &event)
{
   ConnectArgsDialog dlg(this,wxID_ANY,wxT("Connect"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE);

   if(dlg.ShowModal() == wxID_OK){
#ifdef _WIN_
      is_open = serial.Open(dlg.GetDevicePath().wc_str());
#elif _MAC_
      is_open = serial.Open(dlg.GetDevicePath().c_str());
#endif
      serial.SetBaudRate(wxAtoi(dlg.GetBaudRate()));
      serial.SetParity(8,1,'N');

      run_flag = true;

      if(is_open){
	 unsigned char gid[4] = "ACK";
	 serial.Write(gid,4);

	 unsigned char sms[4] = "GOT";
	 serial.Write(sms,4);  
      }
      else{
	 wxMessageBox(wxT("Serial Port Error!"),wxT("Error!"));
      }

   }
}

void Frame::OnKeyDown(wxKeyEvent& event)
{
   //wxMessageBox(wxString::Format("KeyDown: %i\n", (int)event.GetKeyCode()));
   if(event.GetKeyCode() == 32){
      run_flag ^= true;
   }
   event.Skip();
}

void Frame::OnExit(wxCommandEvent &event)
{
   Close();
}

void Frame::BPCalculator()
{

   double as_am_value;
   double ad_am_value;

   ad_am_text->GetValue().ToDouble(&ad_am_value);
   as_am_text->GetValue().ToDouble(&as_am_value);

   double map_ratio = 0;
   uint32_t map_index = 0;
   for(int i = 0;i < pulse_index;++i){
      double temp = ac_array[i] / dc;
      if(temp > map_ratio){
	 map_ratio = temp;
	 MAP = dc_array[i];
	 map_index = i;	       
      }
   }
   if(map_index){
      map_layer_x.push_back(index_array[map_index]);
      map_layer_y.push_back(ac_array[map_index]);
   }

   double dbp_error = 0;
   uint32_t dbp_index = 0;
   for(int i = map_index-1;i >= 0;--i){
      double temp = ac_array[i] / dc;
      double dbp_ratio = temp / map_ratio;
      if(fabs(dbp_ratio - ad_am_value) < dbp_error || dbp_error == 0){
	 dbp_error = fabs(dbp_ratio - ad_am_value);
	 DBP = dc_array[i];
	 dbp_index = i;
      }
   }
   if(dbp_index){
      dbp_layer_x.push_back(index_array[dbp_index]);
      dbp_layer_y.push_back(ac_array[dbp_index]);
   }


   double sbp_error = 0;
   uint32_t sbp_index = 0;
   for(int i = map_index+1;i < pulse_index;++i){
      double temp = ac_array[i] / dc;
      double sbp_ratio = temp / map_ratio;
      if(fabs(sbp_ratio - as_am_value) < sbp_error || sbp_error == 0){
	 sbp_error = fabs(sbp_ratio - as_am_value);
	 SBP = dc_array[i];
	 sbp_index = i;
      }
   }	 
   if(sbp_index){
      sbp_layer_x.push_back(index_array[sbp_index]);
      sbp_layer_y.push_back(ac_array[sbp_index]);
   }

   double pulse_sum = 0;
   double pulse = 0;
   for(int i = 1;i < pulse_index;++i){
      pulse_sum += (1000.0f / ((index_array[i] - index_array[i-1]) * (1000.0f/(double)SAMPLING_RATE)) * 60.0f);
   }	 
   if(pulse_index > 2){
      pulse = pulse_sum / (pulse_index-1);
   }

   if(map_file.is_open()){
      map_file << "DBP" << "," << "MAP" << "," << "SBP" << "," << "Pulse" << std::endl;
      map_file << DBP << "," << MAP << "," << SBP << "," << pulse;
      map_file.close();
   }

   wxString str;
   str.Printf(wxT("%.2f , MAP : %.2f , SBP : %.2f , DBP : %.2f , Pulse : %.2f"), dc , MAP , SBP , DBP , pulse);
   SetStatusText(str);

}

void Frame::OnThreadEvent(wxThreadEvent &event)
{
   const size_t MAX_POINT = 5000;

   if(event.GetInt() == EVT_SERIAL_PORT_READ && is_open){

      unsigned char buffer[3000]={0};
      int length = serial.Read(buffer);

      if(length != -1 && run_flag){

	 wxStringTokenizer tokenizer1(buffer,"R");
	 if(tokenizer1.CountTokens() == 0){
	    return ;
	 }

	 tokenizer1.GetNextToken();

	 while(tokenizer1.HasMoreTokens()){
	    wxString split = tokenizer1.GetNextToken();

	    wxStringTokenizer tokenizer2(split,",");
	    if(tokenizer2.CountTokens() < 5){
	       continue;
	    }	    

	    input_time = std::chrono::steady_clock::now();

	    ++sampling_rate;
	    if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - sampling_time).count() > std::chrono::steady_clock::period::den){
	       wxLogDebug(wxT("Sampling_Rate: %d"),sampling_rate);
	       sampling_rate = 0;
	       sampling_time = std::chrono::steady_clock::now();
	    }	    

	    int token_index = 0;
	    while(tokenizer2.HasMoreTokens()){
	       wxString str = tokenizer2.GetNextToken();       
	       if(token_index == 1){

		  double value = wxAtoi(str);

		  if(init_baseline_count < calibration_point){
		     baseline += value;
		     ++init_baseline_count;
		  }
		  else{
		     if(!first_point_flag){
			first_point_flag = true;
			++folder_counter;
#ifdef _WIN_ 
			time_t t;
			tm *now;
			char name_now[32];
			time(&t); 
			now = localtime(&t); 
			strftime(name_now, 32, "%a %B %d %Hh_%Mm_%Ss", now);
			wxFileName name_path;
			wxString folder_name(name_now + wxString::Format(wxT(" Record,%i"),folder_counter+1));
			name_path.Mkdir(folder_name);  
			record_file.open((const wchar_t*)(folder_name + std::string("/record") + std::string(".csv")), std::fstream::out);
			map_file.open((const wchar_t*)(folder_name + std::string("/map") + std::string(".csv")), std::fstream::out);
			ac_peak_file.open((const wchar_t*)(folder_name + std::string("/ac_peak") + std::string(".csv")), std::fstream::out);
#elif _MAC_
			wxFileName name_path;
			wxString folder_name(wxDateTime::Now().Format().mb_str() + wxString::Format(wxT(" Record,%i"),folder_counter+1));
			name_path.Mkdir(folder_name); 
			record_file.open(folder_name + std::string("/record") + std::string(".csv"),std::fstream::out);
			map_file.open(folder_name + std::string("/map") + std::string(".csv"),std::fstream::out);
			ac_peak_file.open(folder_name + std::string("/ac_peak") + std::string(".csv"),std::fstream::out);			
#endif			
		     }
		     wxString str;

		     //value = fir2.filter(value);
		     value = FIR_filter(value,&info2);
		     dc = baseline / calibration_point;
		     double value_dc = value - dc;
		     dc_layer_x.push_back( time_to_index );

		     //double dc_pressure = (-0.000009f*(value_dc*value_dc))+(0.1361f*value_dc) - 0.661f; // 78base
		     //double dc_pressure = (-0.000001f*(value_dc*value_dc))+(0.13333f*value_dc) - 0.3395f; // 121base
		     //double dc_pressure = (-0.000001f*(value_dc*value_dc))+(0.1263f*value_dc) - 1.0621f; // 174base
		     double dc_pressure = (a[2]*(value_dc*value_dc))+(a[1]*value_dc) - a[0];

		     dc_layer_y.push_back(dc_pressure);
		     //dc_layer_y.push_back(value); // ADC_CH0

		     //dc_layer_y.push_back(value);
		     if (dc_layer_x.size() > MAX_POINT && dc_layer_y.size() > MAX_POINT){
			dc_layer_x.erase(dc_layer_x.begin());
			dc_layer_y.erase(dc_layer_y.begin());
		     } 		     

		     str.Printf(wxT("%.2f mmHg , %.2f , MAP : %.2f"),(dc_pressure), dc , MAP); //v1

		     if(first_point_flag){
			record_file << (((dc_pressure) < 0)?0:(dc_pressure)) << "," << value << ","; //v1
		     }

		     pressure = dc_pressure; //v1
		     SetStatusText(str);
		  }
	       }
	       if(token_index == 2){
		  if(init_baseline_count < calibration_point){
		  }
		  else{
		     ac_layer_x.push_back( time_to_index );
		     double value = FIR_filter(wxAtoi(str),&info1);
		     ac_layer_y.push_back(value);


		     static const float CV_LIMIT = 50.0f;
		     static const float THRESHOLD_FACTOR = 3.0f;
		     double mean = CalculateMean(value);
		     double rms = CalculateRootMeanSquare(value);
		     double cv = CalculateCoefficientOfVariation(value);
		     double threshold;
		     if(cv > CV_LIMIT){
			threshold = dc;
		     }
		     else{
			threshold = (dc * (cv/100.0f) * THRESHOLD_FACTOR);
		     }

		     bool is_peak;
		     SignalPoint result;
		     result = PeakDetect(value,time_to_index,threshold,&is_peak);
		     if(result.index != -1 && (pressure > PRESSURE_MIN && pressure < PRESSURE_MAX)){
			if(is_peak){
			   peak_layer_x.push_back(result.index);
			   peak_layer_y.push_back(result.value);

			   dc_array[pulse_index] = pressure;
			   ac_array[pulse_index] = result.value;
			   index_array[pulse_index] = time_to_index;

			   ac_peak_file << dc_array[pulse_index] << "," << ac_array[pulse_index] << "," << index_array[pulse_index] << std::endl;

			   ++pulse_index;
			}
			else{
			   trough_layer_x.push_back(result.index);
			   trough_layer_y.push_back(result.value);			
			}
		     }		  

		     if(first_point_flag){
			record_file << value << std::endl;
		     }		  
		     //ac_layer_y.push_back(wxAtoi(str));
		     if (ac_layer_x.size() > MAX_POINT && ac_layer_y.size() > MAX_POINT){
			ac_layer_x.erase(ac_layer_x.begin());
			ac_layer_y.erase(ac_layer_y.begin());
			if(peak_layer_x.size()){
			   if((time_to_index - peak_layer_x[0]) > MAX_POINT){
			      peak_layer_x.erase(peak_layer_x.begin());
			      peak_layer_y.erase(peak_layer_y.begin());
			   }
			}
			if(trough_layer_x.size()){
			   if((time_to_index - trough_layer_x[0]) > MAX_POINT){
			      trough_layer_x.erase(trough_layer_x.begin());
			      trough_layer_y.erase(trough_layer_y.begin());
			   }
			}			     
		     } 

		     ++time_to_index;
		  }
	       }		       
	       ++token_index;
	    }
	 }

      }  
   }

   if(event.GetInt() == EVT_REFRESH_PLOT && is_open && run_flag){

      if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - measurment_time).count() > (MEASURMENT_TIME+1) * std::chrono::steady_clock::period::den 
	    || std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - input_time).count() > std::chrono::steady_clock::period::den){
	 BPCalculator();
      }

      if(peak_layer_x.size()){
	 if(!ac_plot->IsLayerVisible(peak_str)){
	    ac_plot->AddLayer(peak_layer);
	 }
	 peak_layer->SetData(peak_layer_x, peak_layer_y);
      }
      else{
	 if(ac_plot->IsLayerVisible(peak_str)){
	    ac_plot->DelLayer(peak_layer);
	 }
      }
      if(trough_layer_x.size()){
	 if(!ac_plot->IsLayerVisible(trough_str)){
	    ac_plot->AddLayer(trough_layer);
	 }	 
	 trough_layer->SetData(trough_layer_x, trough_layer_y);
      }
      else{
	 if(ac_plot->IsLayerVisible(trough_str)){
	    ac_plot->DelLayer(trough_layer);
	 }	 
      }  
      if(map_layer_x.size()){
	 if(!ac_plot->IsLayerVisible(map_str)){
	    ac_plot->AddLayer(map_layer);
	 }
	 map_layer->SetData(map_layer_x, map_layer_y);
      }
      else{
	 if(ac_plot->IsLayerVisible(map_str)){
	    ac_plot->DelLayer(map_layer);
	 }
      }
      if(sbp_layer_x.size()){
	 if(!ac_plot->IsLayerVisible(sbp_str)){
	    ac_plot->AddLayer(sbp_layer);
	 }
	 sbp_layer->SetData(sbp_layer_x, sbp_layer_y);
      }
      else{
	 if(ac_plot->IsLayerVisible(sbp_str)){
	    ac_plot->DelLayer(sbp_layer);
	 }
      }
      if(dbp_layer_x.size()){
	 if(!ac_plot->IsLayerVisible(dbp_str)){
	    ac_plot->AddLayer(dbp_layer);
	 }
	 dbp_layer->SetData(dbp_layer_x, dbp_layer_y);
      }
      else{
	 if(ac_plot->IsLayerVisible(dbp_str)){
	    ac_plot->DelLayer(dbp_layer);
	 }
      }


      dc_layer->SetData(dc_layer_x, dc_layer_y);
      dc_plot->Fit();
      ac_layer->SetData(ac_layer_x, ac_layer_y);
      ac_plot->Fit();      

   }

}

Thread::Thread(Frame *parent,wxEvtHandler *evt):wxThread(wxTHREAD_DETACHED),handler(evt)
{
   frame = parent;
   refresh_last = std::chrono::steady_clock::now();
   read_last = std::chrono::steady_clock::now();
}

void* Thread::Entry()
{
   const int32_t READ_RATE = 100;
   const int32_t FRAME_RATE = 10;

   while(!TestDestroy()){
      if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - read_last).count() > (std::chrono::steady_clock::period::den/READ_RATE)){
	 wxThreadEvent *evt = new wxThreadEvent(wxEVT_THREAD);
	 evt->SetInt(EVT_SERIAL_PORT_READ);
	 handler->QueueEvent(evt);
	 read_last = std::chrono::steady_clock::now();
      }
      if(std::chrono::steady_clock::duration(std::chrono::steady_clock::now() - refresh_last).count() > (std::chrono::steady_clock::period::den/FRAME_RATE)){
	 wxThreadEvent *evt = new wxThreadEvent(wxEVT_THREAD);
	 evt->SetInt(EVT_REFRESH_PLOT);
	 handler->QueueEvent(evt);
	 refresh_last = std::chrono::steady_clock::now();
      }
   }

   return NULL;
}
