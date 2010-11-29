//#include "stdafx.h"

#include "jid.h"
#include "JabberDataBlockListener.h"
#include "..\vs2005\ui\resourceppc.h"
#include "Roster.h"

#include <commctrl.h>
#include <windowsx.h>

#include <utf8.hpp>

#include <algorithm>
#include "boostheaders.h"

#include "Image.h"
#include "JabberAccount.h"
#include "JabberStream.h"
#include "Presence.h"
#include "ProcessMUC.h"

#include "wmuser.h"
#include "TabCtrl.h"
#include "ChatView.h"
#include "ChessView.h"

#include "DlgStatus.h"
#include "DlgAddEditContact.h"
#include "DlglocContact.h"
#include "VcardForm.h"
#include "ClientInfoForm.h"
#include "ServiceDiscovery.h"
#include "MucConfigForm.h"

#include "utf8.hpp"

#include "config.h"

extern TabsCtrlRef tabs;
extern int socketError;
char *NIL="Not-In-List";
#include "mood.h"
#include "activity.h"
extern actParse  *actsParse;
extern  moodParse *moodsParse;
Roster::Roster(ResourceContextRef rc){
    roster=VirtualListView::ref();
    this->rc=rc;
    createGroup("Self-Contact", RosterGroup::SELF_CONTACT);
    createGroup("Transports", RosterGroup::TRANSPORTS);
    createGroup(NIL, RosterGroup::NOT_IN_LIST);

    Contact::ref self=Contact::ref(new Contact(rc->account->getBareJid(), "", "", "",""));
	/*
	Contact::Contact(
	 const std::string &jid,
	 const std::string &resource,
	 const std::string &nickname,
	 const std::string &statusMessage,
	 const std::string &nodeInfo
	)
	*/
    self->group="Self-Contact";
    bareJidMap[self->jid.getBareJid()]=self;
    addContact(self);
}

/*void Roster::addContact(Contact::ref contact) {
    bareJidMap[contact->jid.getBareJid()]=contact;
    contacts.push_back(contact);
}*/

Contact::ref Roster::findContact(const std::string &jid) const {
    Jid right(jid);
    for (ContactList::const_iterator i=contacts.begin(); i!=contacts.end(); i++) {
        Contact::ref r=*i;
        if (r->jid==right) return r;
    }
    return Contact::ref();
}

int Roster::actseticon(const std::string &from,int icon ,const std::string &mess){
for (ContactList::const_iterator i=rc->roster->contacts.begin(); i!=rc->roster->contacts.end(); i++) {
Contact::ref r=*i;
if (r->rosterJid==from){
	r->messact=mess;
	r->acticon=icon;
}}
return 1;
}
int Roster::moodseticon(const std::string &from,int icon ,const std::string &mess){
for (ContactList::const_iterator i=rc->roster->contacts.begin(); i!=rc->roster->contacts.end(); i++) {
Contact::ref r=*i;
if (r->rosterJid==from){
	r->messmood=mess;
	r->moodicon=icon;
}}
return 1;}
int Roster::tuneon(const std::string &from,const std::string &artist,const std::string &title,const std::string &source){
	//Jid right(from);
for (ContactList::const_iterator i=rc->roster->contacts.begin(); i!=rc->roster->contacts.end(); i++) {
Contact::ref r=*i;
if (r->rosterJid==from){r->tuneicon=1;
r->Tartist=artist;
r->Ttitle=title;
r->Tsource=source;
}
}
return 1;}
int Roster::tuneoff(const std::string &from){
	//Jid right(from);
for (ContactList::const_iterator i=rc->roster->contacts.begin(); i!=rc->roster->contacts.end(); i++) {
Contact::ref r=*i;
if (r->rosterJid==from){r->tuneicon=0;
r->Tartist.assign("-");
r->Ttitle.assign("-");
r->Tsource.assign("-");
}
}
return 1;}
ProcessResult Roster::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {

    const std::string & blockTagName=block->getTagName();

    bool rosterPush=(block->getAttribute("type"))=="set";

    JabberDataBlockRef query=block->getChildByName("query");

    if (query.get()==NULL) return BLOCK_REJECTED;
    if (query->getAttribute("xmlns")!="jabber:iq:roster") return BLOCK_REJECTED;

    JabberDataBlockRefList::iterator i=query->getChilds()->begin();
    while (i!=query->getChilds()->end()) {
        JabberDataBlockRef item=*(i++);
        std::string jid=item->getAttribute("jid");
        std::string name=item->getAttribute("name");
        //todo: контакт в нескольких группах
        JabberDataBlockRef jGroup=item->getChildByName("group"); 
        std::string group=(jGroup)? jGroup->getText() : "";

        /*if (group.length()==0)*/ if (jid.find('@')==std::string.npos) 
            group="Transports";

        std::string subscr=item->getAttribute("subscription");
        
        int offlineIcon=presence::OFFLINE;
        if (subscr=="none") offlineIcon=presence::UNKNOWN;
        if (item->hasAttribute("ask")) if (subscr!="remove"){
            subscr+=',';
            subscr+="ask";
            offlineIcon=presence::ASK;
        }

        if (!findGroup(group)) {
            createGroup(group, RosterGroup::ROSTER);
            std::stable_sort(groups.begin(), groups.end(), RosterGroup::compare );
        }

        //при push модифицировать ВСЕ экземпляры по bareJid
        Contact::ref contact;

        if (rosterPush) {
            int i=0;
            bool found=false;
            while (i!=contacts.size()) {
                Contact::ref right=contacts[i];
                if (right->rosterJid==jid) {
                    if (subscr=="remove") {
                        contacts.erase(contacts.begin()+i);
                        //todo: bareJidMap[contact->jid.getBareJid()]=contact; // now it is null
                        continue;
                    } else {
                        found=true;
                        right->subscr=subscr;
                        right->nickname=name;
                        right->group=group;
                        right->offlineIcon=offlineIcon;
                        right->update();
                    }

                }
                i++;
            }

            if (subscr=="remove") break;
            if (found) break;
        } 

        contact=findContact(jid);
        if (contact==NULL) { 
            contact=Contact::ref(new Contact(jid, "", name,"",""));
			/*
			Contact::Contact(
			  const std::string &jid,
			  const std::string &resource,
			  const std::string &nickname,
			  const std::string &statusMessage,
			  const std::string &nodeInfo
			)
			*/
            //std::wstring rjid=utf8::utf8_wchar(contact->rosterJid);
            //roster->addODR(contact, (i==query->getChilds()->end()));
            if (!bareJidMap[contact->jid.getBareJid()]) {
                bareJidMap[contact->jid.getBareJid()]=contact;
                contacts.push_back(contact);
            }
        }   

        contact->subscr=subscr;
        contact->group=group;
        contact->offlineIcon=offlineIcon;

    }
    if (rosterPush) {
        JabberDataBlock result("iq");
        result.setAttribute("type","result");
        result.setAttribute("id",block->getAttribute("id"));
        rc->jabberStream->sendStanza(result);
    }
    //std::stable_sort(contacts.begin(), contacts.end(), Contact::compare);
    makeViewList();
    return BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////

void Roster::deleteContact(Contact::ref contact) {
    int i=0;
    while (i!=contacts.size()) {
        Contact::ref right=contacts[i];
        if (right->rosterJid==contact->rosterJid) {
            if (right->subscr=="NIL") {
                contacts.erase(contacts.begin()+i);
                //todo: bareJidMap[contact->jid.getBareJid()]=contact; // now it is null
                continue;
            } else {
                right->status=(presence::PresenceIndex) icons::ICON_TRASHCAN_INDEX;
                right->update();
            }

        }
        i++;
    }
    makeViewList();

    if (contact->subscr!="NIL") {
        rosterSet(NULL, contact->rosterJid.c_str(), NULL, "remove");
    }
}

void Roster::rosterSet(const char * nick, const char *jid, const char *group, const char *subscr ) {
    JabberDataBlock iqSet("iq");
    iqSet.setAttribute("type","set");
    iqSet.setAttribute("id","roster_set");
    JabberDataBlockRef qry=iqSet.addChildNS("query", "jabber:iq:roster");
    JabberDataBlockRef item=qry->addChild("item", NULL);

    item->setAttribute("jid", jid);
    if (nick) item->setAttribute("name", nick);
    if (group) item->addChild("group", group);
    if (subscr) item->setAttribute("subscription", subscr);

    rc->jabberStream->sendStanza(iqSet);
};
//////////////////////////////////////////////////////////////////////////
Contact::ref Roster::getContactEntry(const std::string & from){

    //first attempt - if we already have this contact in our list
    Contact::ref contact=findContact(from);
    if (!contact) {
        Jid jid(from);
        //second attempt - search for contact without resource
        contact=findContact(jid.getBareJid());
        if (contact) {
            // store resource
            contact->jid.setResource(jid.getResource());
            contact->update();
            needUpdateView=true;
        } else { 
            //third attempt - clone contact from bareJidMap
            if ( bareJidMap.count(jid.getBareJid())!=0 ) {
                contact=bareJidMap[jid.getBareJid()];
                contact=contact->clone();
                contact->jid.setResource(jid.getResource());
                contact->update();
                contacts.push_back(contact);
                needUpdateView=true;
            } else {

                //finally - based on NOT-IN-LIST policy
                contact=Contact::ref(new Contact(jid.getBareJid(), jid.getResource(), "","",""));
				/*
				Contact::Contact(
				  const std::string &jid,
				  const std::string &resource,
				  const std::string &nickname,
				  const std::string &statusMessage,
				  const std::string &nodeInfo
				)
				*/
                //std::string group="Not-In-List";

                contact->subscr="NIL";
                contact->offlineIcon=presence::ASK;
                contact->group=NIL;
                //bareJidMap[contact->jid.getBareJid()]=contact;
                contacts.push_back(contact);
                needUpdateView=true;
            }
        }
    }
    return contact;
}
//////////////////////////////////////////////////////////////////////////
void Roster::makeViewList() {

    std::stable_sort(
        contacts.begin(), 
        contacts.end(), 
        (Config::getInstance()->sortByStatus) ? Contact::compareKST : Contact::compareKT );

    //ODRSet::ref odrlist=ODRSet::ref(new ODRList());
    //ODRList *list=(ODRList *)(odrlist.get());
    
    ODRList *list=new ODRList(); //АХТУНГ №1

    bool showOfflines=Config::getInstance()->showOfflines;
	if( Config::getInstance()->showGroups) {
    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
       list->push_back(group);
//скрываем комнаты  откуда вышли
MucGroup::ref r= boost::dynamic_pointer_cast<MucGroup>(group);
if(r){
	if(r->room->nUnread<=0 && r->selfContact->status==presence::OFFLINE){list->pop_back();  continue;}
}

        int elementsDisplayed=0;

        if (!group->isExpanded()) continue;

        group->addContacts(list);

        for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
            Contact::ref contact=*ci;

            if (!group->equals(contact->group)) continue;

            // hide offline contacts without new messages.
            // TODO: hide only inactive offlines
			//Evtomax не скрывать транспорты
			if (contact->status==presence::OFFLINE && contact->nUnread==0 && !showOfflines && !(group->type==RosterGroup::TRANSPORTS)) continue;
            elementsDisplayed++;
            list->push_back(contact);
        }
        //removing group header if nothing to display
		if(r)continue;//вдруг в комнате сообщение а ты выпал отуда
        if ( elementsDisplayed==0) { list->pop_back();  continue; } 

    }

    needUpdateView=false;
    //roster->bindODRList(odrlist);
    PostMessage(roster->getHWnd(), WM_VIRTUALLIST_REPLACE, 0, (LPARAM)list); //АХТУНГ №2
    //roster->notifyListUpdate(false);
	}else{
	for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
            Contact::ref contact=*ci;
		if (contact->status==presence::OFFLINE && contact->nUnread==0 && !showOfflines ) continue;
            
            list->push_back(contact);
		 }
	
	for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
       
//скрываем комнаты  откуда вышли
MucGroup::ref r= boost::dynamic_pointer_cast<MucGroup>(group);
if(r){if(r->room->nUnread<=0 && r->selfContact->status==presence::OFFLINE)continue;
list->push_back(group);
list->push_back(r->room);
	
}
	
	}
	PostMessage(roster->getHWnd(), WM_VIRTUALLIST_REPLACE, 0, (LPARAM)list); //АХТУНГ №2
	}

}
StringVectorRef Roster::getRosterGroups() {
    StringVectorRef result=StringVectorRef(new StringVector());

    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
        if (group->type==RosterGroup::ROSTER) result->push_back(group->getName());
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
RosterGroup::ref Roster::findGroup( const std::string &name ) {
    for (GroupList::const_iterator i=groups.begin(); i!=groups.end(); i++) {
        RosterGroup::ref r=*i;
        if (r->equals(name)) return r;
    }
    return RosterGroup::ref();
}

RosterGroup::ref Roster::createGroup( const std::string &name, RosterGroup::Type type ) {

    RosterGroup::ref newGrp=RosterGroup::ref(new RosterGroup(name, type));
    groups.push_back(newGrp);
    return newGrp;
}

void Roster::addGroup( RosterGroup::ref group ) {
    groups.push_back(group);
}

void Roster::addContact( Contact::ref contact ) {
    contacts.push_back(contact);
}
 //отправляет всем конференциям текущий статус, приоритет, текст статуса :)(UFO много чего уже поменял)
void Roster::setMUCStatus(int status ) {    
    for (GroupList::const_iterator i=groups.begin(); i!=groups.end(); i++) {
        MucGroup::ref r= boost::dynamic_pointer_cast<MucGroup>(*i);
	
        if (r && !socketError) {
if((r->selfContact->status)==presence::ONLINE ||(r->selfContact->status)==presence::CHAT || (r->selfContact->status)==presence::AWAY ||(r->selfContact->status)==presence::XA ||(r->selfContact->status)==presence::DND){
			JabberDataBlockRef SendStatus=constructPresence(
				r->selfContact->jid.getJid().c_str(), 
				rc->status, 
				rc->presenceMessage, 
				rc->priority); 
 JabberDataBlockRef xMuc=SendStatus->addnod("c", "http://bombusng-md.googlecode.com");
			rc->jabberStream->sendStanza(SendStatus);
}
		}}
}

void Roster::setStatusByFilter( const std::string & bareJid, int status ) {
    int i=0;
    while (i!=contacts.size()) {
        Contact::ref right=contacts[i];
        if (right->jid.getBareJid()==bareJid) {
            right->status=(presence::PresenceIndex)status;
        }
        i++;
    }
}

void Roster::setAllOffline() {
    int i=0;
    while (i!=contacts.size()) {
        contacts[i]->status=presence::OFFLINE;
        i++;
    }
}


Roster::ContactListRef Roster::getHotContacts() {
    ContactListRef hots=ContactListRef(new ContactList());
    for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
        Contact::ref contact=*ci;
        if (contact->nUnread > 0) {
            hots->push_back(contact);
        }
    }

    //muc rooms
    for (GroupList::const_iterator i=groups.begin(); i!=groups.end(); i++) {
        MucGroup::ref r= boost::dynamic_pointer_cast<MucGroup>(*i);
        if (r) {
            MucRoom::ref room=r->room;
            if (room->nUnread>0)
                hots->push_back(room);
        }
    }

    return hots;
}

Roster::ContactListRef Roster::getGroupContacts( RosterGroup::ref group ) {
    ContactListRef gcl=ContactListRef(new ContactList());
    for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
        Contact::ref contact=*ci;
        if (contact->group==group->getName()) {
            gcl->push_back(contact);
        }
    }
    return gcl;
}

RosterGroup::RosterGroup( const std::string &name, Type type ) {
    groupName=name;
    this->type=type;
    sortKey=utf8::utf8_wchar(name);
    wstr=TEXT("General");    if (name.length()!=0) wstr=sortKey;
    expanded=true;
    init();
}

int RosterGroup::getColor() const { return 0xff0000; }

const wchar_t * RosterGroup::getText() const { return wstr.c_str(); }

int RosterGroup::getIconIndex() const { 
    return (expanded)? 
        icons::ICON_EXPANDED_INDEX : 
        icons::ICON_COLLAPSED_INDEX;
}
bool RosterGroup::compare( RosterGroup::ref left, RosterGroup::ref right ) {
    if (left->type < right->type) return true;
    if (left->type > right->type) return false;
    return (_wcsicmp(left->sortKey.c_str(), right->sortKey.c_str()) < 0);
}


//////////////////////////////////////////////////////////////////////////
RosterListView::RosterListView( HWND parent, const std::string & title ){
    parentHWnd=parent;
    init(); 

    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, presence::OFFLINE));
    cursorPos=ODRRef();//odrlist->front();
    odrlist=ODRListRef(new ODRList());

    ////
}

RosterListView::~RosterListView() {}

void RosterListView::eventOk() {
    if (!cursorPos) return;
    RosterGroup *p = dynamic_cast<RosterGroup *>(cursorPos.get());
    if (p) {
    	p->setExpanded(!p->isExpanded());
        roster.lock()->makeViewList();
    } else {
        OnCommand(RosterListView::OPENCHAT, 0);
    }
}



extern ImgListRef skin;
void RosterListView::Client_klik(Contact::ref focusedContact){
	std::wstring msg2=utf8::utf8_wchar(focusedContact->getClientIdIcon());
                    int result2=MessageBox(
                        getHWnd(), 
                        msg2.c_str(), 
                        TEXT("Клиент"), 
                        MB_OK);
};
void RosterListView::Act_klik(Contact::ref focusedContact){
	std::string  msg1=actsParse->lines[focusedContact->acticon+1][1];
	std::wstring msg2=utf8::utf8_wchar(msg1)+L"\n\""+utf8::utf8_wchar(focusedContact->messact)+ L"\"";
                    int result2=MessageBox(
                        getHWnd(), 
                        msg2.c_str(), 
                        TEXT("Активность"), 
                        MB_OK);
};
void RosterListView::Mood_klik(Contact::ref focusedContact){
	std::string  msg1=moodsParse->lines[focusedContact->moodicon+1][1];
	std::wstring msg2=utf8::utf8_wchar(msg1)+L"\n\""+utf8::utf8_wchar(focusedContact->messmood)+ L"\"";
                    int result2=MessageBox(
                        getHWnd(), 
                        msg2.c_str(), 
                        TEXT("Настроение"), 
                        MB_OK);
};

void RosterListView::Tune_klik(Contact::ref focusedContact){
//нажата нотка
std::wstring msg2=L" артист:"+utf8::utf8_wchar(focusedContact->Tartist)+L" название:"+utf8::utf8_wchar(focusedContact->Ttitle)+L" альбом:"+utf8::utf8_wchar(focusedContact->Tsource);
                    int result2=MessageBox(
                        getHWnd(), 
                        msg2.c_str(), 
                        TEXT("Музыка"), 
                        MB_OK);
}

void RosterListView::is_icon_klik(int x){//нажимаем на иконки в ростере
IconTextElement::ref focused = boost::dynamic_pointer_cast<IconTextElement>(cursorPos);
MucRoom * mr= dynamic_cast<MucRoom *>(cursorPos.get());	
if (!cursorPos || !focused || mr) return ;
Contact * c = dynamic_cast<Contact *>(cursorPos.get());
Contact::ref focusedContact = boost::dynamic_pointer_cast<Contact>(cursorPos);

if(c){
	if(x>0 && x<(skin->getElementWidth()-1)){//нажата иконка статуса
		
		std::wstring msg1=L" ";
		switch (focusedContact->status) 
    {
        case -1:msg1=L"PRESENCE_AUTH";
		break;
		case -2:msg1=L"PRESENCE_AUTH";
		break;
		case -3:msg1=L"PRESENCE_AUTH_REMOVE";
		break;
		case icons::ICON_INVISIBLE_INDEX : msg1=L"INVISIBLE";
		break;
		case icons::ICON_ERROR_INDEX : msg1=L"PRESENCE_ERROR";
		break;
		case icons::ICON_TRASHCAN_INDEX : msg1=L"TRASH";
		break;
		case presence::ONLINE:msg1=L"Online";
		break;
		case presence::CHAT:msg1=L"Free for chat";
		break;
		case presence::AWAY:msg1=L"Away";
		break;
		case presence::XA:msg1=L"Extended Away";
		break;
		case presence::DND:msg1=L"DND";
		break;
		case presence::OFFLINE:msg1=L"OFFLINE";
		break;
		case presence::ASK:msg1=L"ASK";
		break;
		case presence::UNKNOWN:msg1=L"UNKNOWN";
		break;

		}

					std::wstring msg2=L"Статуc контакта "+utf8::utf8_wchar(focusedContact->getName())+ L" :\n"+msg1+L" \""+utf8::utf8_wchar(focusedContact->getStatusMessage())+L"\"";
	int result2=MessageBox(
                        getHWnd(), 
                        msg2.c_str(), 
                        TEXT("Cтатус"), 
                        MB_OK);
	}else {//нажатие на иконки клиента,музыки,настроений,активности
		int cl_bit=0;
		if(focusedContact->clientIcon)cl_bit|=1;
		if(focusedContact->tuneicon)  cl_bit|=2;
		if(focusedContact->moodicon>=0)  cl_bit|=4;
		if(focusedContact->acticon>=0)   cl_bit|=8;
		if(x>(clientRect.right-skin->getElementWidth()+1)){
			switch(cl_bit){
				case 1:
				case 3:
				case 5:
				case 7:
				case 9:
				case 11:
				case 13:
				case 15:
					Client_klik(focusedContact);
					break;
				case 2:
				case 6:
				case 10:
				case 14:
					Tune_klik(focusedContact);
					break;
				case 4:
				case 12:
					Mood_klik(focusedContact);
					break;
				case 8:
					Act_klik(focusedContact);
					break;
			}
		}else
			if(x<(clientRect.right-skin->getElementWidth()) && x>(clientRect.right-2*skin->getElementWidth()+1)){
				switch(cl_bit){
				case 3:
				case 7:
				case 11:
				case 15:
					Tune_klik(focusedContact);
					break;
				case 5:
				case 6:
				case 13:
				case 14:
					Mood_klik(focusedContact);
					break;
				case 9:
				case 10:
				case 12:
					Act_klik(focusedContact);
					break;

				}
		}else
			if(x<(clientRect.right-2*skin->getElementWidth()) && x>(clientRect.right-3*skin->getElementWidth()+1)){
				if(cl_bit== 7 || cl_bit==15)Mood_klik(focusedContact);
				if(cl_bit== 11 || cl_bit==13 || cl_bit==14)Act_klik(focusedContact);
			}
			else if(x<(clientRect.right-3*skin->getElementWidth()) && x>(clientRect.right-4*skin->getElementWidth()+1)){
			if(cl_bit==15)Act_klik(focusedContact);
			}
		
		/*if(x>(clientRect.right-skin->getElementWidth()+1)){
			if(focusedContact->clientIcon){Client_klik(focusedContact);}
			else if(focusedContact->tuneicon){Tune_klik(focusedContact);}
			else if(focusedContact->moodicon){Mood_klik(focusedContact);}
			else if(focusedContact->acticon){Act_klik(focusedContact);}
		}
		else 
			if(x<(clientRect.right-skin->getElementWidth()) && x>(clientRect.right-2*skin->getElementWidth()+1)){
			if(focusedContact->tuneicon && focusedContact->clientIcon){Tune_klik(focusedContact);}
			else if(focusedContact->moodicon &&( focusedContact->tuneicon || focusedContact->clientIcon)){Mood_klik(focusedContact);}
			else if(focusedContact->acticon && (focusedContact->moodicon || focusedContact->tuneicon || focusedContact->clientIcon)){Act_klik(focusedContact);}
			} 
			else
				if(x<(clientRect.right-2*skin->getElementWidth()) && x>(clientRect.right-3*skin->getElementWidth()+1)){
					if(focusedContact->moodicon && focusedContact->tuneicon && focusedContact->clientIcon){Mood_klik(focusedContact);}
					else if(focusedContact->acticon ){Act_klik(focusedContact);}
				}
				else 
					if(x<(clientRect.right-3*skin->getElementWidth()) && x>(clientRect.right-4*skin->getElementWidth()+1)){
					if(focusedContact->acticon ){Act_klik(focusedContact);}
					}*/
}
}}
HMENU RosterListView::getContextMenu() {
    if (!cursorPos) return NULL;

    HMENU hmenu=CreatePopupMenu();

    IconTextElement::ref focused = boost::dynamic_pointer_cast<IconTextElement>(cursorPos);
    if (focused) {
        focused->createContextMenu(hmenu);
    }

    //////////////////////////////////////////////////////////////////////////
    // Group actions
    RosterGroup *rg = dynamic_cast<RosterGroup *>(cursorPos.get());
    if (rg) {
        if (rg->type==RosterGroup::ROSTER)
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::RENAMEGRP,           TEXT("Переименовать"));

    }
    
    Contact * c = dynamic_cast<Contact *>(cursorPos.get());

    if (c) {
        RosterGroup::Type type=roster.lock()->findGroup(c->group)->type;

        MucRoom * mr= dynamic_cast<MucRoom *>(cursorPos.get());

        if (type==RosterGroup::TRANSPORTS) {
            AppendMenu(hmenu, MF_STRING, RosterListView::LOGON,                TEXT("Включить"));
            AppendMenu(hmenu, MF_STRING, RosterListView::LOGOFF,               TEXT("Выключить"));
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::RESOLVENICKNAMES,     TEXT("Получить ники"));
            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
        }
        AppendMenu(hmenu, MF_STRING , RosterListView::OPENCHAT,                TEXT("Открыть чат"));

        AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

        if (!mr) {

            AppendMenu(hmenu, MF_STRING, RosterListView::VCARD,                    TEXT("VCard"));
            AppendMenu(hmenu, MF_STRING, RosterListView::CLIENTINFO,               TEXT("Инфо о клиенте"));

HMENU subscrMenupPEP=CreatePopupMenu();
AppendMenu(subscrMenupPEP, MF_STRING, RosterListView::MUZYKA,                    TEXT("музыка"));
  AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenupPEP,               TEXT("PEP"));

            AppendMenu(hmenu, MF_STRING, RosterListView::COMMANDS,                 TEXT("Команды"));
			AppendMenu(hmenu, MF_STRING, RosterListView::LOCMES,          TEXT("Послать локацию"));
            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

            if (type==RosterGroup::ROSTER) {
                AppendMenu(hmenu, MF_STRING, RosterListView::EDITCONTACT,          TEXT("Изменить контакт"));
                
                HMENU subscrMenu=CreatePopupMenu();

                AppendMenu(subscrMenu, MF_STRING, RosterListView::SUBSCRIBE, TEXT("Запросить подписку"));
                AppendMenu(subscrMenu, MF_STRING, RosterListView::SUBSCRIBED, TEXT("Предоставить подписку"));
                AppendMenu(subscrMenu, MF_STRING, RosterListView::UNSUBSCRIBED, TEXT("Отозвать подписку"));

                AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenu,               TEXT("Подписка"));


            }

            if (type==RosterGroup::NOT_IN_LIST)
                AppendMenu(hmenu, MF_STRING, RosterListView::ADDCONTACT,           TEXT("Добавить контакт"));

            if (type!=RosterGroup::MUC && type!=RosterGroup::SELF_CONTACT) {
                AppendMenu(hmenu, MF_STRING, RosterListView::DELETECONTACT,            TEXT("Удалить"));

                AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

                AppendMenu(hmenu, MF_STRING, RosterListView::SENDSTATUS,               TEXT("Отправить статус"));
            }
            if (type!=RosterGroup::TRANSPORTS) {
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::SENDFILE,             TEXT("Отправить файл"));
				//Скрыто значит..
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::INVITE,               TEXT("Пригласить"));
            }
        }

        if (mr) {
            if (mr->status!=presence::OFFLINE) {
                AppendMenu(hmenu, MF_STRING, RosterListView::SENDSTATUS,               TEXT("Отправить статус"));
                AppendMenu(hmenu, MF_STRING , RosterListView::MUC_LEAVE,   TEXT("Покинуть комнату"));
            }
            else 
                AppendMenu(hmenu, MF_STRING , RosterListView::MUC_REENTER,  TEXT("Перезайти в комнату"));

            MucGroup::ref roomGrp;
            roomGrp=boost::dynamic_pointer_cast<MucGroup> (roster.lock()->findGroup(mr->group));
            MucContact::Role myRole=roomGrp->selfContact->role;
            MucContact::Affiliation myAff=roomGrp->selfContact->affiliation;
            if (myAff==MucContact::OWNER || myAff==MucContact::ADMIN) {
                AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::MLOUTCASTS,           TEXT("Забаненные"));
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::MLMEMBERS,            TEXT("Участники"));
            }
            if (myAff==MucContact::OWNER) {
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::MLADMINS,             TEXT("Администраторы"));
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterListView::MLOWNERS,             TEXT("Владельцы"));
                AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
                AppendMenu(hmenu, MF_STRING, RosterListView::MUCCONFIG,               TEXT("Настроить комнату"));
            }
        }

        MucContact * mc= dynamic_cast<MucContact *>(cursorPos.get());
        if (mc) {
            MucGroup::ref roomGrp;
            roomGrp=boost::dynamic_pointer_cast<MucGroup> (roster.lock()->findGroup(mc->group));
            BOOST_ASSERT(roomGrp);

            MucContact::Role myRole=roomGrp->selfContact->role;
            MucContact::Affiliation myAff=roomGrp->selfContact->affiliation;

            bool online=mc->status!=presence::OFFLINE;

            bool canKick = (myRole==MucContact::MODERATOR && mc->affiliation <MucContact::ADMIN && online);
            bool canBan = 
                myAff==MucContact::OWNER || 
                (myAff==MucContact::ADMIN 
                && (mc->affiliation==MucContact::NONE || mc->affiliation==MucContact::MEMBER));


            if (!canKick && !canBan) return hmenu;
            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
            if (canKick) AppendMenu(hmenu, MF_STRING, RosterListView::MUCKICK, L"Выкинуть");
            if (canBan)  AppendMenu(hmenu, MF_STRING, RosterListView::MUCBAN,  L"Забанить");

            if (canKick) {
                HMENU roleMenu=CreatePopupMenu();
                AppendMenu(roleMenu, MF_STRING | (mc->role==MucContact::VISITOR ? MF_CHECKED : 0), RosterListView::MUCVISITOR, L"Гость");
                AppendMenu(roleMenu, MF_STRING | (mc->role==MucContact::PARTICIPANT ? MF_CHECKED : 0), RosterListView::MUCPARTICIPANT, L"Посетитель");
                AppendMenu(roleMenu, MF_STRING | (mc->role==MucContact::MODERATOR ? MF_CHECKED : 0), RosterListView::MUCMODERATOR, L"Модератор");

                AppendMenu(hmenu, MF_POPUP, (LPARAM)roleMenu,               TEXT("Роль"));
            }

            if (myAff>=MucContact::ADMIN) {
                HMENU afflMenu=CreatePopupMenu();
                bool hasItems=false;
                if (myAff==MucContact::OWNER || myAff>=mc->affiliation) {
                    AppendMenu(afflMenu, MF_STRING | (mc->affiliation==MucContact::NONE ? MF_CHECKED : 0), RosterListView::MUCNONE, L"Никто");
                    AppendMenu(afflMenu, MF_STRING | (mc->affiliation==MucContact::MEMBER ? MF_CHECKED : 0), RosterListView::MUCMEMBER, L"Участник");
                    hasItems=true;
                }
                if (myAff==MucContact::OWNER) {
                    AppendMenu(afflMenu, MF_STRING | (mc->affiliation==MucContact::ADMIN ? MF_CHECKED : 0), RosterListView::MUCADMIN, L"Администратор");
                    AppendMenu(afflMenu, MF_STRING | (mc->affiliation==MucContact::OWNER ? MF_CHECKED : 0), RosterListView::MUCOWNER, L"Владелец");
                    //hasItems=true; // rudimentary here
                }

                //finally
                if (hasItems)
                    AppendMenu(hmenu, MF_POPUP, (LPARAM)afflMenu,               TEXT("Звание"));
                else 
                    DestroyMenu(afflMenu);
            }

        }

        if (c->enableServerHistory!=Contact::DISABLED_STATE) {
            HMENU sshMenu=CreatePopupMenu();
            AppendMenu(sshMenu, MF_STRING | (c->enableServerHistory==Contact::DEFAULT ? MF_CHECKED : 0), 
                RosterListView::SSH_DEFAULT, TEXT("Как в настройках"));
            AppendMenu(sshMenu, MF_STRING | (c->enableServerHistory==Contact::BLOCK ? MF_CHECKED : 0), 
                RosterListView::SSH_DISABLED, TEXT("Выключена"));
            AppendMenu(sshMenu, MF_STRING | (c->enableServerHistory==Contact::ALLOW ? MF_CHECKED : 0), 
                RosterListView::SSH_ENABLED, TEXT("Включена"));

            AppendMenu(hmenu, MF_POPUP, (LPARAM)sshMenu,               TEXT("История"));

        }

		AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hmenu, MF_STRING, RosterListView::OPENCHESS, L"Шахматы");
    }

    return hmenu;
}

void RosterListView::OnCommand( int cmdId, LONG lParam ) {
    if (roster.expired()) return;
    ResourceContextRef rc=roster.lock()->rc;

    IconTextElement::ref focused = boost::dynamic_pointer_cast<IconTextElement>(cursorPos);
    if (focused) focused->onCommand(cmdId, rc);

    Contact::ref focusedContact = boost::dynamic_pointer_cast<Contact>(cursorPos);

    if (focusedContact) {
        switch (cmdId) {
        case RosterListView::OPENCHAT: 
            {
                openChat(focusedContact);
                break;
            }
		case RosterListView::MUZYKA:
			{Tune_klik(focusedContact);
			break;}
        case RosterListView::LOGON: 
            //
        case RosterListView::LOGOFF: 
            rc->sendPresence(
                focusedContact->jid.getJid().c_str(), 
                (cmdId==RosterListView::LOGON)? presence::ONLINE: presence::OFFLINE, 
                rc->presenceMessage, rc->priority );
            break;

        case RosterListView::RESOLVENICKNAMES:
            break;

        case RosterListView::VCARD: 
            {
                if (!rc->isLoggedIn()) break;
                WndRef vc=VcardForm::createVcardForm(tabs->getHWnd(), focusedContact->rosterJid, rc, false);
                tabs->addWindow(vc);
                tabs->switchByWndRef(vc);
                break;
            }
        case RosterListView::CLIENTINFO: 
            {
                if (!rc->isLoggedIn()) break;
                const std::string &jid=(focusedContact->status==presence::OFFLINE)?
                    focusedContact->rosterJid : focusedContact->jid.getJid();
                WndRef ci=ClientInfoForm::createInfoForm(tabs->getHWnd(), jid, rc);
                tabs->addWindow(ci);
                tabs->switchByWndRef(ci);
                break;
            }
        case RosterListView::COMMANDS:
            {
                ServiceDiscovery::ref disco=ServiceDiscovery::createServiceDiscovery(
                    tabs->getHWnd(), 
                    rc, 
                    focusedContact->jid.getJid(), 
                    "http://jabber.org/protocol/commands", 
                    true);
                tabs->addWindow(disco);
                tabs->switchByWndRef(disco);
            }
            break;
            //case RosterView::SUBSCR: 
        case RosterListView::SUBSCRIBE:
        case RosterListView::SUBSCRIBED: 
        case RosterListView::UNSUBSCRIBED: 
            {
                presence::PresenceIndex subscr=presence::PRESENCE_AUTH_ASK;
                if (cmdId==RosterListView::SUBSCRIBED) subscr=presence::PRESENCE_AUTH;
                if (cmdId==RosterListView::UNSUBSCRIBED) subscr=presence::PRESENCE_AUTH_REMOVE;
                rc->sendPresence(focusedContact->rosterJid.c_str(),
                    subscr, std::string(), 0);
                break;
            }

        case RosterListView::EDITCONTACT:
        case RosterListView::ADDCONTACT:
            DlgAddEditContact::createDialog(getHWnd(), rc, focusedContact); break;
		case RosterListView::LOCMES:
			DlglocContact::createDialog(getHWnd(), rc, focusedContact); break;

        case RosterListView::DELETECONTACT:
            {
                std::wstring name=utf8::utf8_wchar(focusedContact->getFullName());
                int result=MessageBox(getHWnd(), name.c_str(), TEXT("Удалить контакт ?"), MB_YESNO | MB_ICONWARNING);
                if (result==IDYES) {
                    roster.lock()->deleteContact(focusedContact);
                }
                break;
            }
        case RosterListView::SENDSTATUS:
            DlgStatus::createDialog(getHWnd(), rc, focusedContact); break;

		case RosterListView::MUC_REENTER:
		case RosterListView::MUC_LEAVE:
			rc->sendPresence(focusedContact->rosterJid.c_str(), (cmdId==RosterListView::MUC_LEAVE)?presence::OFFLINE:presence::ONLINE, std::string(), 0);
			break;

        case RosterListView::SENDFILE: 
        case RosterListView::INVITE:
            break;

        case RosterListView::MUCCONFIG:
            {
                MucConfigForm::ref mucconf=MucConfigForm::createMucConfigForm(
                    tabs->getHWnd(), 
                    focusedContact->jid.getBareJid(), 
                    rc);
                tabs->addWindow(mucconf);
                tabs->switchByWndRef(mucconf);
                break;
            }

        case RosterListView::MUCVISITOR:
        case RosterListView::MUCPARTICIPANT:
        case RosterListView::MUCMODERATOR:
        case RosterListView::MUCKICK: 
            {
                MucContact::Role role=MucContact::VISITOR;
                if (cmdId==MUCMODERATOR) role=MucContact::MODERATOR;
                if (cmdId==MUCPARTICIPANT) role=MucContact::PARTICIPANT;
                if (cmdId==MUCKICK) {
                    std::wstring name=utf8::utf8_wchar(focusedContact->getFullName());
                    int result=MessageBox(getHWnd(), name.c_str(), TEXT("Выкинуть ?"), MB_YESNO | MB_ICONWARNING);
                    if (result==IDNO) break;
                    role=MucContact::NONE_ROLE;
                }

                MucContact::ref mc=boost::dynamic_pointer_cast<MucContact>(focusedContact);
                if (mc) mc->changeRole(rc, role);
                break;
            }

        case RosterListView::MUCNONE:
        case RosterListView::MUCMEMBER:
        case RosterListView::MUCADMIN:
        case RosterListView::MUCOWNER:
        case RosterListView::MUCBAN: 
            {
                // because of security reasons here may be up to 2 questions:
                // - modifying ownership
                // - banning an occupant
                MucContact::Affiliation newAffiliation=MucContact::OUTCAST;
                if (cmdId==MUCNONE) newAffiliation=MucContact::NONE;
                if (cmdId==MUCMEMBER) newAffiliation=MucContact::MEMBER;
                if (cmdId==MUCADMIN) newAffiliation=MucContact::ADMIN;
                if (cmdId==MUCOWNER) newAffiliation=MucContact::OWNER;

                MucContact::ref mc=boost::dynamic_pointer_cast<MucContact>(focusedContact);
                if (!mc) break;
                if (newAffiliation==mc->affiliation) break;

                if (mc->affiliation==MucContact::OWNER || newAffiliation==MucContact::OWNER) {
                    char *fmt="Вы действительно хотите лишить %s привилегий владельца?";
                    if (newAffiliation==MucContact::OWNER) fmt=
                        "Вы действительно хотите предоставить %s привилегии владельца?\n"
                        "ВНИМАНИЕ!!! Привилегия владельца - самая высокая привилегия в конференции!";

                    std::wstring msg=utf8::utf8_wchar(boost::str(boost::format(fmt) % focusedContact->getFullName()));
                    int result=MessageBox(
                        getHWnd(), 
                        msg.c_str(), 
                        TEXT("Изменение привилегий владельца"), 
                        MB_YESNO | MB_ICONWARNING);
                    if (result==IDNO) break;
                }

                std::wstring name=utf8::utf8_wchar(focusedContact->getFullName());

                if (newAffiliation==MucContact::OUTCAST) {
                    int result=MessageBox(getHWnd(), name.c_str(), TEXT("Забанить???"), MB_YESNO | MB_ICONWARNING);
                    if (result==IDNO) break;
                }

                mc->changeAffiliation(rc, newAffiliation);
                break;
            }
		case RosterListView::OPENCHESS:
			openChess(focusedContact);
			break;
        default:
            break;
        }
    }

    if (cmdId==ID_JABBER_ADDACONTACT){
        DlgAddEditContact::createDialog(getHWnd(), rc, Contact::ref());
    }
}
bool RosterListView::showWindow( bool show ) {
    Wnd::showWindow(show);
    if (show) SetFocus(getHWnd());
    return false;
}
void RosterListView::setIcon( int iconIndex ) {
    wt->setIcon(iconIndex);
    InvalidateRect(tabs->getHWnd(), NULL, false);
}

void RosterListView::openChat(Contact::ref contact) {
    WndRef chat=tabs->getWindowByODR(contact);
    if (!chat) {
        //Contact::ref r=roster.lock()->findContact(c->jid.getJid());
        chat=WndRef(new ChatView(tabs->getHWnd(), contact));
        tabs->addWindow(chat);
    }
    tabs->switchByWndRef(chat);
}

void RosterListView::openChess(Contact::ref contact)
{
	WndRef chat=tabs->getWindowByODR(contact);
	if (!chat) {
		chat=WndRef(new ChessView(tabs->getHWnd(), contact));

	    tabs->addWindow(chat);
    }
	tabs->switchByWndRef(chat);
}

#ifdef ENABLE_ROSTER_VIEW
//////////////////////////////////////////////////////////////////////////
ATOM RosterView::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ChatView::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusRV");

    return RegisterClass(&wc);
}

//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK RosterView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    HDC hdc;
    RosterView *p=(RosterView *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(RosterView *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            p->rosterListView=VirtualListView::ref(new VirtualListView(hWnd, std::string("Roster")));
            p->rosterListView->setParent(hWnd);
            p->rosterListView->showWindow(true);
            p->rosterListView->wrapList=true;
            p->rosterListView->colorInterleaving=true;

            //p->msgList->bindODRList(p->contact->messageList);
            break;
        }

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        {
            //p->contact->nUnread=0;
            RECT rc = {0, 0, 200, tabHeight};
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, p->contact->getColor());
            p->contact->draw(hdc, rc);

            int iconwidth= skin->getElementWidth();
            skin->drawElement(hdc, 0, /*icons::ICON_CLOSE*/, p->width-2-iconwidth, 0);
            //skin->drawElement(hdc, icons::ICON_TRASHCAN_INDEX, p->width-2-iconwidth*2, 0);

            /*SetBkMode(hdc, TRANSPARENT);
            LPCTSTR t=p->title.c_str();
            DawrText(hdc, t, -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            DrwText(hdc, t, -1, &rc, DT_LEFT | DT_TOP);*/
        }

        EndPaint(hWnd, &ps);
        break;

        //case WM_KILLFOCUS:
        //    p->contact->nUnread=0;
        //    break;

    case WM_SIZE: 
        { 
            HDWP hdwp; 
            RECT rc; 

            int height=GET_Y_LPARAM(lParam);
            p->width=GET_X_LPARAM(lParam);

            int ySplit=height-p->editHeight;

            p->calcEditHeight();

            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&rc, 0, 0, 
                GET_X_LPARAM(lParam), ySplit ); 

            // Size the tab control to fit the client area. 
            hdwp = BeginDeferWindowPos(2);

            /*DeferWindowPos(hdwp, dropdownWnd, HWND_TOP, 0, 0, 
            GET_X_LPARAM(lParam), 20, 
            SWP_NOZORDER 
            ); */


            DeferWindowPos(hdwp, p->msgList->getHWnd(), HWND_TOP, 0, tabHeight, 
                GET_X_LPARAM(lParam), ySplit-tabHeight, 
                SWP_NOZORDER 
                );
            /*DeferWindowPos(hdwp, rosterWnd, HWND_TOP, 0, tabHeight, 
            GET_X_LPARAM(lParam), height-tabHeight, 
            SWP_NOZORDER 
            );*/

            DeferWindowPos(hdwp, p->editWnd, NULL, 0, ySplit+1, 
                GET_X_LPARAM(lParam), height-ySplit-1, 
                SWP_NOZORDER 
                ); 

            EndDeferWindowPos(hdwp); 

            break; 
        } 

    case WM_COMMAND: 
        {
            if (wParam==IDS_SEND) {
                p->sendJabberMessage();
            }
            if (wParam==IDC_COMPLETE) {
                p->mucNickComplete();
            }

            if (wParam==IDC_COMPOSING) {
                p->setComposingState(lParam!=0);
            }
            break;             
        }
        /*case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: 
        {
        //HGDIOBJ brush= GetStockObject(GRAY_BRUSH);
        //HGDIOBJ pen= GetStockObject(WHITE_PEN);
        SetBkColor(hdc, 0x808080);
        SetTextColor(hdc, 0xffffff);
        //SelectObject((HDC)wParam, brush);
        //SelectObject((HDC)wParam, pen);
        return (BOOL) GetStockObject(GRAY_BRUSH);
        break;
        }*/

    case WM_LBUTTONDOWN:
        SetFocus(hWnd);
        if ((GET_Y_LPARAM(lParam))>tabHeight) break;

        if (GET_X_LPARAM(lParam) > (p->width)-2-(skin->getElementWidth()) ) {
            PostMessage(GetParent(hWnd), WM_COMMAND, TabsCtrl::CLOSETAB, 0);
            break;
        }
        if (GET_X_LPARAM(lParam) > (p->width)-2-(skin->getElementWidth())*2) {
            int result=MessageBox(
                p->getHWnd(), 
                L"Are You sure want to clear this chat session?", 
                L"Clear chat", 
                MB_YESNO | MB_ICONWARNING);
            if (result==IDYES) {
                p->contact->messageList->clear();
                p->msgList->moveCursorEnd();
            }
            break;
        }
        break;

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

#endif