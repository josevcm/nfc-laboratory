#include "lms7002_dlgVCOfrequencies.h"
#include <wx/filedlg.h>
#include <vector>
#include <fstream>
#include "lms7_device.h"

using namespace std;
using namespace lime;

lms7002_dlgVCOfrequencies::lms7002_dlgVCOfrequencies( wxWindow* parent , lms_device_t* plmsControl)
: wxDialog( parent, wxID_ANY, wxT("VCO frequencies") )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxFlexGridSizer* fgSizer247;
    fgSizer247 = new wxFlexGridSizer( 0, 1, 5, 5 );
    fgSizer247->SetFlexibleDirection( wxBOTH );
    fgSizer247->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxFlexGridSizer* fgSizer240;
    fgSizer240 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer240->SetFlexibleDirection( wxBOTH );
    fgSizer240->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    wxStaticBoxSizer* sbSizer129;
    sbSizer129 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCOH SXR/SXT") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer241;
    fgSizer241 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer241->SetFlexibleDirection( wxBOTH );
    fgSizer241->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText341 = new wxStaticText( sbSizer129->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText341->Wrap( -1 );
    fgSizer241->Add( m_staticText341, 0, wxALL, 5 );
    
    txtVCOH_low = new wxTextCtrl( sbSizer129->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOH_low->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOH_low->SetMaxLength( 10 );
    }
    #else
    txtVCOH_low->SetMaxLength( 10 );
    #endif
    txtVCOH_low->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer241->Add( txtVCOH_low, 0, 0, 5 );
    
    m_staticText342 = new wxStaticText( sbSizer129->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText342->Wrap( -1 );
    fgSizer241->Add( m_staticText342, 0, wxALL, 5 );
    
    txtVCOH_high = new wxTextCtrl( sbSizer129->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOH_high->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOH_high->SetMaxLength( 10 );
    }
    #else
    txtVCOH_high->SetMaxLength( 10 );
    #endif
    txtVCOH_high->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer241->Add( txtVCOH_high, 0, 0, 5 );
    
    
    sbSizer129->Add( fgSizer241, 1, wxEXPAND, 5 );
    
    
    fgSizer240->Add( sbSizer129, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer1291;
    sbSizer1291 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCOM SXR/SXT") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer2411;
    fgSizer2411 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer2411->SetFlexibleDirection( wxBOTH );
    fgSizer2411->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText3411 = new wxStaticText( sbSizer1291->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3411->Wrap( -1 );
    fgSizer2411->Add( m_staticText3411, 0, wxALL, 5 );
    
    txtVCOM_low = new wxTextCtrl( sbSizer1291->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOM_low->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOM_low->SetMaxLength( 10 );
    }
    #else
    txtVCOM_low->SetMaxLength( 10 );
    #endif
    txtVCOM_low->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer2411->Add( txtVCOM_low, 0, 0, 5 );
    
    m_staticText3421 = new wxStaticText( sbSizer1291->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3421->Wrap( -1 );
    fgSizer2411->Add( m_staticText3421, 0, wxALL, 5 );
    
    txtVCOM_high = new wxTextCtrl( sbSizer1291->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOM_high->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOM_high->SetMaxLength( 10 );
    }
    #else
    txtVCOM_high->SetMaxLength( 10 );
    #endif
    txtVCOM_high->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer2411->Add( txtVCOM_high, 0, 0, 5 );
    
    
    sbSizer1291->Add( fgSizer2411, 1, wxEXPAND, 5 );
    
    
    fgSizer240->Add( sbSizer1291, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer1292;
    sbSizer1292 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCOL SXR/SXT") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer2412;
    fgSizer2412 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer2412->SetFlexibleDirection( wxBOTH );
    fgSizer2412->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText3412 = new wxStaticText( sbSizer1292->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3412->Wrap( -1 );
    fgSizer2412->Add( m_staticText3412, 0, wxALL, 5 );
    
    txtVCOL_low = new wxTextCtrl( sbSizer1292->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOL_low->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOL_low->SetMaxLength( 10 );
    }
    #else
    txtVCOL_low->SetMaxLength( 10 );
    #endif
    txtVCOL_low->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer2412->Add( txtVCOL_low, 0, 0, 5 );
    
    m_staticText3422 = new wxStaticText( sbSizer1292->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3422->Wrap( -1 );
    fgSizer2412->Add( m_staticText3422, 0, wxALL, 5 );
    
    txtVCOL_high = new wxTextCtrl( sbSizer1292->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOL_high->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOL_high->SetMaxLength( 10 );
    }
    #else
    txtVCOL_high->SetMaxLength( 10 );
    #endif
    txtVCOL_high->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer2412->Add( txtVCOL_high, 0, 0, 5 );
    
    
    sbSizer1292->Add( fgSizer2412, 1, wxEXPAND, 5 );
    
    
    fgSizer240->Add( sbSizer1292, 1, wxEXPAND, 5 );
    
    wxStaticBoxSizer* sbSizer1293;
    sbSizer1293 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("VCO CGEN") ), wxVERTICAL );
    
    wxFlexGridSizer* fgSizer2413;
    fgSizer2413 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer2413->SetFlexibleDirection( wxBOTH );
    fgSizer2413->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    m_staticText3413 = new wxStaticText( sbSizer1293->GetStaticBox(), wxID_ANY, wxT("Low(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3413->Wrap( -1 );
    fgSizer2413->Add( m_staticText3413, 0, wxALL, 5 );
    
    txtVCOCGEN_low = new wxTextCtrl( sbSizer1293->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOCGEN_low->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOCGEN_low->SetMaxLength( 10 );
    }
    #else
    txtVCOCGEN_low->SetMaxLength( 10 );
    #endif
    txtVCOCGEN_low->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer2413->Add( txtVCOCGEN_low, 0, 0, 5 );
    
    m_staticText3423 = new wxStaticText( sbSizer1293->GetStaticBox(), wxID_ANY, wxT("High(GHz):"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3423->Wrap( -1 );
    fgSizer2413->Add( m_staticText3423, 0, wxALL, 5 );
    
    txtVCOCGEN_high = new wxTextCtrl( sbSizer1293->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    #ifdef __WXGTK__
    if ( !txtVCOCGEN_high->HasFlag( wxTE_MULTILINE ) )
    {
    txtVCOCGEN_high->SetMaxLength( 10 );
    }
    #else
    txtVCOCGEN_high->SetMaxLength( 10 );
    #endif
    txtVCOCGEN_high->SetMinSize( wxSize( 64,-1 ) );
    
    fgSizer2413->Add( txtVCOCGEN_high, 0, 0, 5 );
    
    
    sbSizer1293->Add( fgSizer2413, 1, wxEXPAND, 5 );
    
    
    fgSizer240->Add( sbSizer1293, 1, wxEXPAND, 5 );
    
    
    fgSizer247->Add( fgSizer240, 1, wxEXPAND, 5 );
    
    wxFlexGridSizer* fgSizer249;
    fgSizer249 = new wxFlexGridSizer( 0, 4, 5, 5 );
    fgSizer249->SetFlexibleDirection( wxBOTH );
    fgSizer249->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    
    btnOk = new wxButton( this, wxID_ANY, wxT("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( btnOk, 0, 0, 5 );
    
    btnCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( btnCancel, 0, 0, 5 );
    
    btnLoadFile = new wxButton( this, wxID_ANY, wxT("Load file"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( btnLoadFile, 0, 0, 5 );
    
    btnSaveFile = new wxButton( this, wxID_ANY, wxT("Save to file"), wxDefaultPosition, wxDefaultSize, 0 );
    fgSizer249->Add( btnSaveFile, 0, 0, 5 );
    
    
    fgSizer247->Add( fgSizer249, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
    
    
    this->SetSizer( fgSizer247 );
    this->Layout();
    fgSizer247->Fit( this );
    
    this->Centre( wxBOTH );
    
    // Connect Events
    btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_dlgVCOfrequencies::OnBtnOkClick ), NULL, this );
    btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_dlgVCOfrequencies::OnBtnCancelClick ), NULL, this );
    btnLoadFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_dlgVCOfrequencies::OnLoadFile ), NULL, this );
    btnSaveFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( lms7002_dlgVCOfrequencies::OnSaveFile ), NULL, this );

    lmsControl = plmsControl;
    double multiplier = 0.000000001;
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();

    txtVCOL_low->SetValue(wxString::Format(_("%.3f"), lms->gVCO_frequency_table[0][0] * multiplier));
    txtVCOL_high->SetValue(wxString::Format(_("%.3f"), lms->gVCO_frequency_table[0][1] * multiplier));
    txtVCOM_low->SetValue(wxString::Format(_("%.3f"), lms->gVCO_frequency_table[1][0] * multiplier));
    txtVCOM_high->SetValue(wxString::Format(_("%.3f"), lms->gVCO_frequency_table[1][1] * multiplier));
    txtVCOH_low->SetValue(wxString::Format(_("%.3f"), lms->gVCO_frequency_table[2][0] * multiplier));
    txtVCOH_high->SetValue(wxString::Format(_("%.3f"), lms->gVCO_frequency_table[2][1] * multiplier));
    txtVCOCGEN_low->SetValue(wxString::Format(_("%.3f"), lms->gCGEN_VCO_frequencies[0] * multiplier));
    txtVCOCGEN_high->SetValue(wxString::Format(_("%.3f"), lms->gCGEN_VCO_frequencies[1] * multiplier));
}

void lms7002_dlgVCOfrequencies::OnBtnOkClick( wxCommandEvent& event )
{
    double multiplier = 1e9;
    double value;
    LMS7002M* lms = ((LMS7_Device*)lmsControl)->GetLMS();

    txtVCOL_low->GetValue().ToDouble(&value);
    lms->gVCO_frequency_table[0][0]=value*multiplier;
    txtVCOL_high->GetValue().ToDouble(&value);
    lms->gVCO_frequency_table[0][1]=value*multiplier;

    txtVCOM_low->GetValue().ToDouble(&value);
    lms->gVCO_frequency_table[1][0]=value*multiplier;
    txtVCOM_high->GetValue().ToDouble(&value);
    lms->gVCO_frequency_table[1][1]=value*multiplier;

    txtVCOH_low->GetValue().ToDouble(&value);
    lms->gVCO_frequency_table[2][0]=value*multiplier;
    txtVCOH_high->GetValue().ToDouble(&value);
    lms->gVCO_frequency_table[2][1]=value*multiplier;

    txtVCOCGEN_low->GetValue().ToDouble(&value);
    lms->gCGEN_VCO_frequencies[0] = value*multiplier;
    txtVCOCGEN_high->GetValue().ToDouble(&value);
    lms->gCGEN_VCO_frequencies[1] = value*multiplier;

    EndModal(wxID_OK);
}

void lms7002_dlgVCOfrequencies::OnBtnCancelClick( wxCommandEvent& event )
{
    EndModal(wxID_CANCEL);
}

void lms7002_dlgVCOfrequencies::OnSaveFile(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Save vco file"), "", "", "text (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    fstream fout;
    fout.open(dlg.GetPath().mb_str(), ios::out);

    fout << "[VCOH] count: " << 2 << "," << endl;
    fout << txtVCOH_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOH_high->GetValue() << ' ' << "GHz," << endl;

    fout << "[VCOM] count: " << 2 << "," << endl;
    fout << txtVCOM_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOM_high->GetValue() << ' ' << "GHz," << endl;

    fout << "[VCOL] count: " << 2 << "," << endl;
    fout << txtVCOL_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOL_high->GetValue() << ' ' << "GHz," << endl;

    fout << "[VCO_CGEN] count: " << 2 << "," << endl;
    fout << txtVCOCGEN_low->GetValue() << ' ' << "GHz," << endl;
    fout << txtVCOCGEN_high->GetValue() << ' ' << "GHz," << endl;
    fout.close();
}

void lms7002_dlgVCOfrequencies::OnLoadFile(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Open vco file"), "", "", "text (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    fstream fin;
    fin.open(dlg.GetPath().ToStdString().c_str(), ios::in);

    char cline[128];
    int lineLen = 128;

    char vconame[80];
    char ctemp[80];
    double freq = 0;
    int freqCount = 0;

    std::vector<double> frequencies;

    while (!fin.eof())
    {
        freqCount = 0;
        fin.getline(cline, lineLen);
        //find name and frequency count
        sscanf(cline, "%s %s %i", vconame, ctemp, &freqCount);
        if (strcmp(vconame, "[VCOH]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOH_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOH_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
        if (strcmp(vconame, "[VCOM]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOM_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOM_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
        if (strcmp(vconame, "[VCOL]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOL_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOL_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
        if (strcmp(vconame, "[VCO_CGEN]") == 0)
        {
            frequencies.clear();
            for (int i = 0; i<freqCount; ++i)
            {
                fin.getline(cline, lineLen);
                sscanf(cline, "%lf %s", &freq, ctemp);
                frequencies.push_back(freq);
            }
            if (frequencies.size() >= 2)
            {
                txtVCOCGEN_low->SetValue(wxString::Format(_("%.3f"), frequencies[0]));
                txtVCOCGEN_high->SetValue(wxString::Format(_("%.3f"), frequencies[frequencies.size() - 1]));
            }
        }
    }
    fin.close();
}
