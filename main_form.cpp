//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main_form.h"
#include <vcl.Clipbrd.hpp>
#include "tiles.h"
#include <System.JSON.Types.hpp>
#include <System.JSON.Writers.hpp>
#include <System.JSON.Builders.hpp>
#include <System.Classes.hpp>

static constexpr const char * CONFIG_FILE = 		"config.json";
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner), consoleVisible(false)
{
	exitDialog = new TTaskDialog(this);
	exitDialog->Title = "Confirm";
	exitDialog->Caption = "Close or hide?";

	exitDialog->ModalResult = mrOk;
	exitDialog->Flags = TTaskDialogFlags() << tfAllowDialogCancellation;
    exitDialog->CommonButtons = TTaskDialogCommonButtons();

	exitDialog->Buttons->Clear();

	TTaskDialogBaseButtonItem *button = exitDialog->Buttons->Add();
	button->Caption = "Close";
	button->ModalResult = mrOk;

	button = exitDialog->Buttons->Add();
	button->Caption = "Hide";
	button->ModalResult = mrNo;

	trayIcon->Visible = true;

	loadBtnClick(Owner);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::onActivate(TObject *Sender)
{
	linkEdit->Text = Clipboard()->AsText;
}
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	Application->OnActivate = &this->onActivate;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::linkEditDblClick(TObject *Sender)
{
	linkEdit->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::updateStateOfPict(TImage * component)
{
	Context * ctx = reinterpret_cast<Context*>(component->Tag);
	if(ctx->isEnded())
	{
		if(ctx->getLastError().isEmpty())
			component->Picture = doneHolderImg->Picture;
		else
			component->Picture = errorHolderImg->Picture;
	}
}
void __fastcall TMainForm::updateTimerTimer(TObject *Sender)
{
	for(int i = 0; i < statusFlow->ComponentCount; i++)
	{
		auto c = statusFlow->Components[i];
		updateStateOfPict(static_cast<TImage*>(c));
	}
}
//---------------------------------------------------------------------------

class StatusContext : public Context
{
	int status;
	public:
	StatusContext()
	{
		status = rand() % 10;
	}
	virtual ~StatusContext()
	{       }
	virtual void runCommand(String str)
	{
        status--;
    }
	virtual bool isEnded() const
	{
		return status <= 0;
    }
	virtual Optional<String> waitTillEnd()
	{
		status = 0;
		return Optional<String>{NullOption()};
    }
	virtual Optional<String> getLastError()
	{
		return Optional<String>{NullOption()};
    }
};

void __fastcall TMainForm::createStateImg(Context *ctx)
{
	TImage *img = new TImage(statusFlow);

	img->Parent = statusFlow;
	img->Width = 24;
	img->Height = 24;
	img->AlignWithMargins = true;
	img->Picture = workingHolderImg->Picture;
	img->Transparent = true;
	img->Margins = workingHolderImg->Margins;
	img->Tag = reinterpret_cast<NativeInt>(ctx);
	img->OnDblClick = &onStateDblClick;
	img->OnMouseUp = &onStateMouseUp;
}
void __fastcall TMainForm::Button2Click(TObject *Sender)
{
	auto ctx = new StatusContext;
    createStateImg(ctx);
}
void __fastcall TMainForm::onStateMouseUp(TObject *Sender, TMouseButton btn, TShiftState Shift, int X, int Y)
{
	if(btn == TMouseButton::mbRight)
	{
		createConsole();
		TImage * component = static_cast<TImage*>(Sender);
		Context * ctx = reinterpret_cast<Context*>(component->Tag);
        ctx->pipeToStdout();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::onStateDblClick(TObject *Sender)
{
	TImage * component = static_cast<TImage*>(Sender);
	Context * ctx = reinterpret_cast<Context*>(component->Tag);
	if(ctx->isEnded())
	{
		if(ctx->getLastError().isEmpty())
		{
			delete ctx;
			component->Tag = 0;
			delete component;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::createConsole()
{
	if(consoleVisible)
		return;
	consoleVisible = true;
	AllocConsole();
	HWND console = GetConsoleWindow();
	if( HMENU hMenu = GetSystemMenu( console, FALSE ) )
	{
		EnableMenuItem( hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );
	}
	SetConsoleCtrlHandler(nullptr, true);
	freopen_s(&consoleFp, "CONOUT$", "w", stdout);
 //	Application->MessageBox(SysErrorMessage(GetLastError()).c_str(), L"Caption");
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::svgDblClick(TObject *Sender)
{
	auto * tile = reinterpret_cast<class Tile*>(static_cast<TSkSvg*>(Sender)->Tag);
	createStateImg(tile->run(linkEdit->Text));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::svgClick(TObject *Sender)
{
	auto * tile = reinterpret_cast<class Tile*>(static_cast<TSkSvg*>(Sender)->Tag);
	imageEdit->Text = tile->getImage();
    commandEdit->Text = tile->getCommand();
	sampleSvg->Svg->Source = imageEdit->Text;
	sampleSvg->Tag = reinterpret_cast<NativeInt>(tile);
	updateBtn->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Button5Click(TObject *Sender)
{
	if(!consoleVisible)
		return;
	fclose(consoleFp);
	consoleFp = nullptr;
	consoleVisible = false;
	FreeConsole();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::saveBtnClick(TObject *Sender)
{
//	if(!consoleVisible)
//		createConsole();
//	std::cout << "Console: " << std::endl;
//	TStringWriter * stringWriter = new TStringWriter();
	TFileStream *output = new TFileStream(CONFIG_FILE, fmCreate);
	TJsonTextWriter * writer = new TJsonTextWriter(output);
	writer->Formatting = TJsonFormatting::Indented;
	writer->WriteStartObject();
	writer->WritePropertyName("tiles");
	writer->WriteStartArray();
	for(int i = 0; i < tilesPanel->ControlCount; i++)
	{
		auto c = tilesPanel->Controls[i];
		auto * svg = static_cast<TSkSvg*>(c);
		class Tile * tile = reinterpret_cast<class Tile *>(svg->Tag);
		writer->WriteStartObject();
		writer->WritePropertyName("type");
		writer->WriteValue("tile");
		writer->WritePropertyName("image");
		writer->WriteValue(tile->getImage());
		writer->WritePropertyName("command");
		writer->WriteValue(tile->getCommand());
		writer->WriteEndObject();
	}
	writer->WriteEndArray();
	writer->WriteEndObject();
//	String str = stringWriter->ToString();
//	std::wcout << (const wchar_t*)str.FirstChar() << std::endl;
//	delete stringWriter;
	delete writer;
	delete output;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::loadBtnClick(TObject *Sender)
{
	if(!FileExists(CONFIG_FILE))
	{
		TResourceStream *resource = new TResourceStream(reinterpret_cast<unsigned int>(GetModuleHandle(NULL)), L"CONFIG_SAMPLE", RT_RCDATA);
		resource->SaveToFile(CONFIG_FILE);
		delete resource;
		Application->MessageBox(L"New empty configuration file (config.json) was created\nPlease configurate application", L"Information");
	}
	TFileStream *input = new TFileStream(CONFIG_FILE, fmOpenRead);
	TTextReader *textReader = new TStreamReader(input);
	TJsonTextReader *reader = new TJsonTextReader(textReader);
	TJSONIterator *json = new TJSONIterator(reader);

	if(json->Next("tiles") && json->Recurse())
	{
		for (int i = tilesPanel->ControlCount - 1; i >= 0; i--)
		{
			TControl* control = tilesPanel->Controls[i];
			delete control;
		}
		updateBtn->Enabled = false;
        sampleSvg->Tag = 0;
		while(json->Next() && json->Recurse())
		{
			json->Next("type");
			if(json->AsString != String{"tile"})
			{
				json->Return();
                continue;
			}
			json->Next("image");
			String img = json->AsString;
			json->Next("command");
			String command = json->AsString;
			class Tile * tile = new class Tile(img, command);
			TSkSvg *svg = new TSkSvg(tilesPanel);
			tile->component = svg;

			svg->Parent = tilesPanel;
			svg->Width = 81;
			svg->Height = 81;
			svg->OnDblClick = svgDblClick;
			svg->OnClick = svgClick;

			svg->Tag = reinterpret_cast<NativeInt>(tile);
			svg->Svg->Source = tile->getImage();

			json->Return();
        }
	}

	delete json;
	delete reader;
    delete textReader;
	delete input;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::imageEditChange(TObject *Sender)
{
	sampleSvg->Svg->Source = imageEdit->Text;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::addBtnClick(TObject *Sender)
{
	class Tile * tile = new class Tile(imageEdit->Text, commandEdit->Text);
	TSkSvg *svg = new TSkSvg(tilesPanel);
    tile->component = svg;

	svg->Parent = tilesPanel;
	svg->Width = 81;
	svg->Height = 81;
	svg->OnDblClick = svgDblClick;
	svg->OnClick = svgClick;

	svg->Tag = reinterpret_cast<NativeInt>(tile);
	svg->Svg->Source = tile->getImage();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::trayIconClick(TObject *Sender)
{
    this->Hide();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::trayIconDblClick(TObject *Sender)
{
	this->Show();
	Application->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	exitDialog->Execute();
	if(exitDialog->ModalResult != mrOk)
		CanClose = false;
	if(exitDialog->ModalResult == mrNo)
        this->Hide();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::updateBtnClick(TObject *Sender)
{
	if(sampleSvg->Tag == 0)
		return;
	class Tile * tile = reinterpret_cast<class Tile *>(sampleSvg->Tag);
	TSkSvg * svg = static_cast<TSkSvg*>(tile->component);
	tile->setImage(imageEdit->Text);
    tile->setCommand(commandEdit->Text);
	svg->Svg->Source = tile->getImage();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::splitCloseBtnClick(TObject *Sender)
{
	configSptil->Close();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::splitOpenBtnClick(TObject *Sender)
{
	configSptil->Open();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::configSptilClosing(TObject *Sender)
{
	commandEdit->Enabled = false;
	imageEdit->Enabled = false;
	imgText = imageEdit->Text;
	cmdText = commandEdit->Text;
	imageEdit->Text = "";
    commandEdit->Text = "";
}
void __fastcall TMainForm::configSptilClosed(TObject *Sender)
{
	imageEdit->Text = imgText;
	commandEdit->Text = cmdText;
}
void __fastcall TMainForm::configSptilOpening(TObject *Sender)
{
	imgText = imageEdit->Text;
	cmdText = commandEdit->Text;
	imageEdit->Text = "";
    commandEdit->Text = "";
}
void __fastcall TMainForm::configSptilOpened(TObject *Sender)
{
	imageEdit->Enabled = true;
	commandEdit->Enabled = true;
	imageEdit->Text = imgText;
	commandEdit->Text = cmdText;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::splitAnimateChkClick(TObject *Sender)
{
    configSptil->UseAnimation = splitAnimateChk->Checked;
}
//---------------------------------------------------------------------------

