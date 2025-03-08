//---------------------------------------------------------------------------

#ifndef main_formH
#define main_formH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.WinXCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <System.Skia.hpp>
#include <Vcl.Skia.hpp>
#include <System.ImageList.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Dialogs.hpp>
#include "tiles.h"
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TSplitView *configSptil;
	TFlowPanel *tilesPanel;
	TPanel *Panel1;
	TTrayIcon *trayIcon;
	TMemo *linkEdit;
	TGroupBox *GroupBox1;
	TCheckBox *destinationAskChk;
	TFlowPanel *statusFlow;
	TImage *errorHolderImg;
	TImage *workingHolderImg;
	TImage *doneHolderImg;
	TButton *Button2;
	TButton *Button5;
	TButton *saveBtn;
	TButton *addBtn;
	TTimer *updateTimer;
	TMemo *commandEdit;
	TMemo *imageEdit;
	TSkSvg *sampleSvg;
	TButton *loadBtn;
	TButton *updateBtn;
	TSpeedButton *splitCloseBtn;
	TSpeedButton *splitOpenBtn;
	TCheckBox *splitAnimateChk;
	void __fastcall FormCreate(TObject *Sender);
    void __fastcall onActivate(TObject *Sender);
	void __fastcall linkEditDblClick(TObject *Sender);
	void __fastcall updateTimerTimer(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall saveBtnClick(TObject *Sender);
	void __fastcall loadBtnClick(TObject *Sender);
	void __fastcall imageEditChange(TObject *Sender);
	void __fastcall addBtnClick(TObject *Sender);
	void __fastcall trayIconClick(TObject *Sender);
	void __fastcall trayIconDblClick(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall updateBtnClick(TObject *Sender);
	void __fastcall splitCloseBtnClick(TObject *Sender);
	void __fastcall splitOpenBtnClick(TObject *Sender);
	void __fastcall configSptilClosing(TObject *Sender);
	void __fastcall configSptilClosed(TObject *Sender);
	void __fastcall configSptilOpening(TObject *Sender);
	void __fastcall configSptilOpened(TObject *Sender);
	void __fastcall splitAnimateChkClick(TObject *Sender);

private:	// User declarations
	FILE * consoleFp;
	void __fastcall updateStateOfPict(TImage * component);
	void __fastcall onStateMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall onStateDblClick(TObject *Sender);
	void __fastcall createStateImg(Context *ctx);
	void __fastcall svgDblClick(TObject *Sender);
	void __fastcall svgClick(TObject *Sender);
	void __fastcall createConsole();
	bool consoleVisible;
	TTaskDialog *exitDialog;
    String imgText, cmdText;
public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
