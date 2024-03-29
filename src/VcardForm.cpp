#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "wmuser.h"
#include "VcardForm.h"
#include "JabberStream.h"

#include "utf8.hpp"
#include "base64.h"
#include "../gsgetfile/include/gsgetlib.h"
#include "config.h"
#include "ResourceContext.h"
#include "Roster.h"
extern std::wstring appRootPath;

wchar_t * ext[]={L"*.unknown", L"*.jpg", L"*.png", L"*.gif", L"*.bmp"};
char *mime[]={"image/unknown", "image/gpeg", "image/png", "image/gif", "image/x-ms-bmp"};

#define FILEIMGMASK L"Image files (*.png; *.jpg; *.gif; *.bmp)\0*.PNG;*.JPG;*.GIF;*.BMP\0All flles (*.*)\0*.*\0\0\0"

class GetVcard : public JabberDataBlockListener {
public:
    GetVcard(const std::string &jid, VcardForm::ref form);
    ~GetVcard(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc);
private:
    std::string jid;
    std::string id;
    ResourceContextRef rc;
    boost::weak_ptr<VcardForm> vf;
};

GetVcard::GetVcard(const std::string &jid, VcardForm::ref form){
    this->jid=jid;
    this->vf=form;
    id="vf#";
    id+=jid;
}

void GetVcard::doRequest(ResourceContextRef rc) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "get");
    req.setAttribute("id", id);

    req.addChildNS("vCard", "vcard-temp");

    rc->jabberStream->sendStanza(req);
}


/*
std::string  WStringToString(const std::wstring& s)
{
    std::string temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp; 
}
*/

std::wstring imgFile;


void VcardForm::onWmUserUpdate() {
    //std::string xml=*(vcard->toXML());
    //const std::string &xml2=XMLStringPrep(xml);
    //const std::wstring xml3=utf8::utf8_wchar(xml2);

    //SendMessage(hwndHTML, WM_SETTEXT, 0, (LPARAM)"");

    //img.reset();

	imgFile=appRootPath;// + L"userdata\\avatars\\" + L"$$$_tmp.jpg";

	if (savePhoto(imgFile.c_str())){
        //img=ImageRef(new Image(imgFile.c_str()));
		//Contact::ref contact = Contact::ref(new Contact(this->jid,"","","",""));
		//contact->img_avatar=img;
	}


    startHtml();
    addImg(L"\\vcard");
    beginForm("vcard-photo", "photo");
    if (img) if (img->getHBmp()) {
        button("save",std::string("Save")); 
        if (editForm) button("clear",std::string("Clear")); 
    }
    if (editForm) button("load",std::string("Load")); 
    addText("<BR>");
    endForm(); 

	//contact->img_avatar=img;
    

    beginForm("vcard-fields", "publish");

    addHtmlField("FN", NULL,        "Full Name", TXT);
    addHtmlField("NICKNAME", NULL,  "Nickname", TXT);
    addHtmlField("BDAY", NULL,      "Birthday", TXT);
    addHtmlField("ADR", "STREET",   "Street", TXT);
    addHtmlField("ADR", "EXTADR",   "Street2", TXT);
    addHtmlField("ADR", "LOCALITY", "City", TXT);
    addHtmlField("ADR", "REGION",   "State", TXT);
    addHtmlField("ADR", "PCODE",    "Post code", TXT);
    addHtmlField("ADR", "CTRY",     "Country", TXT);
    addHtmlField("TEL", "HOME",     "Home Phone Number", TXT);
    addHtmlField("TEL", "NUMBER",   "Phone Number", TXT);
    addHtmlField("EMAIL", "USERID", "E-Mail", URL);
    addHtmlField("TITLE", NULL,     "Position", TXT);
    addHtmlField("ROLE", NULL,      "Role", TXT);
    addHtmlField("ORG", "ORGNAME",  "Organization", TXT);
    addHtmlField("ORG", "ORGUNIT",  "Dept", TXT);
    addHtmlField("URL", NULL,       "Url", URL);
    addHtmlField("DESC", NULL,      "About", MULTILINE);

    if (editForm) button(std::string("Publish"));
    //button("reload",std::string("Reload")); 
    endForm();

    endHtml();
}

ProcessResult GetVcard::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    VcardForm::ref vfRef=vf.lock();
    if (!vfRef) return CANCEL;

    vfRef->vcardArrivedNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
VcardForm::ref VcardForm::createVcardForm( HWND parent,const std::string &jid, ResourceContextRef rc, bool edit) {
    VcardForm *vf=new VcardForm();

    vf->editForm=edit;
    vf->parentHWnd=parent;
    vf->title=utf8::utf8_wchar(jid);
    vf->wt=WndTitleRef(new WndTitle(vf, icons::ICON_VCARD));

    vf->init();
    
    SetParent(vf->thisHWnd, parent);

	vf->rc=rc;

    VcardForm::ref vfRef=VcardForm::ref(vf);

    GetVcard *getv=new GetVcard(jid, vfRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getv));
    getv->doRequest(rc);

    return vfRef;
}

void VcardForm::vcardArrivedNotify(JabberDataBlockRef vcard){
    this->vcard=vcard;

    PostMessage(getHWnd(), WM_HTML_UPDATE, 0, (LPARAM)"");
}

void VcardForm::addHtmlField( const char *ns1, const char *ns2, const char *description, int flags ) {
    std::string value;
    if (vcard) {
        JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard", "vcard-temp");      
        if (vcardTemp) {
            JabberDataBlockRef field=vcardTemp->getChildByName(ns1);     
            if (field) {
                if (ns2) field=field->getChildByName(ns2);
            }
            if (field) value=XMLStringPrep(field->getText());
        }
    }

    if (!editForm) if (value.length()==0) return;

    std::string name(ns1);
    if (ns2) {
        name+="#";
        name+=ns2;
    }
    const std::wstring wname=utf8::utf8_wchar(name);

    if (editForm) {
        if (flags==TXT) flags=TEXTBOX;
        if (flags==URL) flags=TEXTBOX;
    } else {
        if (flags==MULTILINE) flags=TXT;
    }

    if (flags & TEXTBOX) {
        textBox(name.c_str(), std::string(description), value);
        return;
    }
    if (flags & MULTILINE) {
        textML(name.c_str(), std::string(description), value);
        return;
    }

    if (flags & URL)   {
        url(std::string(description), value);
        return;
    }

    if (!editForm) if (value.length()==0) return;

    textConst(std::string(description), value);
}

HBITMAP VcardForm::getImage( LPCTSTR url, DWORD cookie ) {
    this->cookie=cookie;
    if (img) return img->getHBmp();
    return NULL;
}

extern HINSTANCE g_hInst;



std::wstring toWString2(const std::string& s)
{
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp; 
}



BOOL VcardForm::savePhoto( LPCTSTR path ) 
{
    if (!vcard) return FALSE;

    JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard", "vcard-temp");      if (!vcardTemp) return FALSE;
    JabberDataBlockRef photo=vcardTemp->getChildByName("PHOTO");    if (!photo) return FALSE;
    JabberDataBlockRef binval=photo->getChildByName("BINVAL"); if (!binval) return FALSE;
    const std::string &data=binval->getText();

    int dstLen=base64::base64DecodeGetLength(data.length());
    char *dst=new char[dstLen];

    dstLen=base64::base64Decode2(dst, data.c_str(), data.length());

    wchar_t filename[MAX_PATH];
    if (!path) {
        wchar_t *extm=ext[detectMime(dst)]; 
        *filename=0;
        wcscpy_s(filename, MAX_PATH, extm);
        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(OPENFILENAME));
        ofn.lStructSize=sizeof(OPENFILENAME);
        ofn.hwndOwner=getHWnd();         
        ofn.hInstance=g_hInst; //GetModuleHandle(NULL);  
        ofn.lpstrFilter=FILEIMGMASK;     
        ofn.nFilterIndex=0;
        ofn.lpstrFile=filename;     
        ofn.nMaxFile=MAX_PATH;
        //ofn.lpstrInitialDir   = ;     
        ofn.lpstrTitle=L"Save image as";
        ofn.lpstrDefExt=extm+2;
        ofn.Flags=OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

        BOOL result = GetSaveFileNameEx(&ofn);
        if (!result) return FALSE;
        path=filename;
    }

	std::string vcard_to = vcard->getAttribute("from");
size_t i=0;
                while (i<vcard_to.length()) {
                    if (vcard_to[i]=='/') {
                       vcard_to[i]='.';
                        continue;
                    }
                    i++;
				}
	//Log::getInstance()->msg("[***]CASHED Image OK("+vcard_from.substr(0,vcard_from.find("/",0))+")");
	std::wstring new_path = appRootPath + L"userdata\\avatars\\" + utf8::utf8_wchar(vcard_to) + L".jpg";
    std::wstring imageFile=appRootPath + L"userdata\\avatars\\" + utf8::utf8_wchar(vcard_to) + L".jpg";
	//int result=MessageBox(NULL, new_path.c_str(), TEXT("123"), MB_YESNO | MB_ICONWARNING );
	LPCTSTR path_save=imageFile.c_str();
	if(Config::getInstance()->isLOG)Log::getInstance()->msg("[***Get.]("+vcard_to+")");

    HANDLE file=CreateFile(path_save,
        GENERIC_WRITE, 
        FILE_SHARE_READ, NULL, 
        CREATE_ALWAYS,
        0, NULL);

    DWORD dwProcessed;
    if (file==INVALID_HANDLE_VALUE) {
        delete[] dst; return FALSE;
    }
    WriteFile(file, dst, dstLen, &dwProcessed, NULL);
    CloseHandle(file);


	//�������� �����
	Contact::ref contact;
	 contact=rc.lock()->roster->getContactEntry(vcard_to);
	 contact->img_avatar.reset();
	 contact->img_avatar=ImageRef(new Image(imageFile.c_str()));
	 img.reset();
	 img=ImageRef(new Image(imageFile.c_str()));
	if(Config::getInstance()->isLOG)Log::getInstance()->msg("[***SUCCESS.]("+contact->statusMessage+")");
	//�������� � ��������


	/*
	  void Log::msg(const std::string &message){
	    addLog(utf8::utf8_wchar(message).c_str());
	  }
	  void Log::addLog(const wchar_t * msg) {}
	*/
	delete[] dst;
    return TRUE;
}


void VcardForm::onHotSpot( LPCSTR url, LPCSTR param ) {
    std::string nurl=std::string(url);
    if (nurl=="load") {
        wchar_t filename[MAX_PATH];
        *filename=0;
        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(OPENFILENAME));
        ofn.lStructSize=sizeof(OPENFILENAME);
        ofn.hwndOwner=getHWnd();
        ofn.hInstance=g_hInst;
        ofn.lpstrFilter=FILEIMGMASK;
        ofn.nFilterIndex=0;
        ofn.lpstrFile=filename;
        ofn.nMaxFile=MAX_PATH;
        ofn.lpstrTitle=L"Select image file";
        ofn.Flags=OFN_FILEMUSTEXIST | OFN_EXPLORER;
        BOOL result=GetOpenFileNameEx(&ofn);
        if (!result) return;

        loadPhoto(filename);

        vcardArrivedNotify(vcard);
        return;
    }
    if (nurl=="save") {
        savePhoto(NULL);
        return;
    }
    if (nurl=="clear") {
        //todo: clear photo
        JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard","vcard-temp");
        if (!vcardTemp) return;
        vcardTemp->removeChild("PHOTO");
        vcardArrivedNotify(vcard);
        return;
    }
    if (nurl=="reload") {
        //todo: re-request vcard
        return;
    }
    if (nurl=="publish") {
        //todo: publish vcard

        
        JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard","vcard-temp");
        if (!vcardTemp) {
            vcardTemp=JabberDataBlockRef(new JabberDataBlock("vCard"));
            vcardTemp->setAttribute("xmlns","vcard-temp");
        }

        JabberDataBlock viq("iq");
        viq.setAttribute("to",vcard->getAttribute("from"));
        viq.setAttribute("type","set");
        viq.setAttribute("id","my-vcard-publish");
        viq.addChild(vcardTemp);

        StringMapRef m=HtmlView::splitHREFtext(param);
        for (StringMap::iterator i=m->begin(); i!=m->end(); i++) {
            std::string key=i->first;
            std::string value=i->second;
            std::string key2;
            int k2=key.find('#');
            if (k2>0) {
                key2=key.substr(k2+1, key.length()-k2-1);
                key=key.substr(0, k2);
            }

            JabberDataBlockRef child=vcardTemp->getChildByName(key.c_str());
            if (!child) child=vcardTemp->addChild(key.c_str(), NULL);
            if (k2>0) {
                JabberDataBlockRef child2=child->getChildByName(key2.c_str());
                if (!child2) child2=child->addChild(key2.c_str(), NULL);
                child=child2;
            }
            child->setText(value);
        }

        /*if (img) if (img->getHBmp()) {
            std::wstring imgFile=appRootPath+L"$tmpimg.jpg";
            loadPhoto(imgFile.c_str());
        }*/

        //StringRef result=viq->toXML();

        //todo: verify result
        rc.lock()->jabberStream->sendStanza(viq);
        
        vcardArrivedNotify(vcard);
        return;
    }

}

void VcardForm::loadPhoto( LPCTSTR path ) {
    if (!vcard) return;
    JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard","vcard-temp");

    HANDLE file=CreateFile(path, 
        GENERIC_READ, 
        FILE_SHARE_READ, NULL, 
        OPEN_EXISTING,
        0, NULL);

    DWORD dwProcessed;
    if (file!=INVALID_HANDLE_VALUE) {
        DWORD size=GetFileSize(file, NULL);
        char *dst=new char[size];
        ReadFile(file, dst, size, &dwProcessed, NULL);
        CloseHandle(file);

        DWORD encLength=base64::base64EncodeGetLength(size);
        char *enc=new char[encLength];
        encLength=base64::base64Encode(enc, dst, size);

        //todo: get mime-type
        delete[] dst;
        vcardTemp->removeChild("PHOTO");
        JabberDataBlockRef photo=vcardTemp->addChild("PHOTO", NULL);
        photo->addChild("BINVAL",enc);
        photo->addChild("TYPE",mime[detectMime(dst)]);
        delete[] enc;
    }
}

int VcardForm::detectMime( char *photo ) {
    if (photo[0]==(char)0xff &&
        photo[1]==(char)0xd8 &&
        photo[6]==(char)'J' &&
        photo[7]==(char)'F' &&
        photo[8]==(char)'I' &&
        photo[9]==(char)'F')
        return 1; //"image/jpeg";

    if (photo[0]==(char)0x89 &&
        photo[1]==(char)'P' &&
        photo[2]==(char)'N' &&
        photo[3]==(char)'G')
        return 2; //"image/png";

    if (photo[0]==(char)'G' &&
        photo[1]==(char)'I' &&
        photo[2]==(char)'F')
        return 3; //"image/gif";

    if (photo[0]==(char)'B' &&
        photo[1]==(char)'M')
        return 4; //"image/x-ms-bmp";
    return 0;
}

