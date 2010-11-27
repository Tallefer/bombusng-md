//#include "stdafx.h"

#include "Jid.h"
#include "Contact.h"
#include "utf8.hpp"
#include "Image.h"
#include "TimeFunc.h"
#include "stringutils.h"
#include "config.h"//Подключаем конфиг
#include "Log.h"
extern std::string statusMessage2;
extern std::string typevs;
extern DWORD COLORS[];
//////////////////////////////////////////////////////////////////////////
extern ImgListRef skin;

//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////

int identifyTransport(const std::string &jid) {
    int serv=jid.find('@')+1;
    int dotsrv=jid.find('.', serv);
    if (dotsrv<0) return 0;

    Skin * il= dynamic_cast<Skin *>(skin.get());
    if (!il) return 0;
    return il->getBaseIndex(jid.substr(serv, dotsrv-serv));
}
//////////////////////////////////////////////////////////////////////////

Contact::Contact(const std::string &jid,
				 const std::string &resource,
				 const std::string &nickname,
				 const std::string &statusMessage,
				 const std::string &nodeInfo
				 )
{
    this->jid=Jid(jid, resource);
    this->rosterJid=jid;
    this->nickname=nickname;
	this->statusMessage=statusMessage;
	this->nodeInfo=nodeInfo;
    this->status=presence::OFFLINE;
    offlineIcon=presence::OFFLINE;

    enableServerHistory=DISABLED_STATE;

    nUnread=0;
    sortKey=0;

    transpIndex=identifyTransport(jid);

    composing=false;
    acceptComposing=false;

    update();
    messageList=ODRListRef(new ODRList);
}

Contact::ref Contact::clone() {
    Contact::ref c=Contact::ref(new Contact(jid.getBareJid(), jid.getResource(), nickname,statusMessage,nodeInfo));
    c->subscr=subscr;
    c->group=group;
    c->offlineIcon=offlineIcon;
    c->enableServerHistory=enableServerHistory;
    return c;
}

/*bool Contact::hasUnreadMsgs() {
    for ( ODRList::const_iterator i=messageList->begin();   i!=messageList->end();  i++ ) {
        ODRRef r=*i;
        Message *m = dynamic_cast<Message *>(r.get());
        if(m) {
        	if (m->unread) return true;
        }
    }
    return false;
}*/


int Contact::getColor() const{
	//return 0; 
	return (status>7)?0:COLORS[status];
}

/*
void Contact::draw(HDC hdc, RECT &rt) const {
	//SetBkMode(hdc, TRANSPARENT);
	//RECT rc = {0, 0, 200, tabHeight};
	int iconIdx=getIconIndex();
	if (iconIdx>=0) {
		skin->drawElement(hdc, getIconIndex(), rt.left, rt.top);
		rt.left+=skin->getElementWidth()+ICON_SPACING;//Ширина элемента
	} else rt.left+=1;
	//ExtTextOut(hdc, xbegin, ypos, ETO_CLIPPED, &rt, lineBegin, end-lineBegin, NULL);
	//ExtTextOut(hdc, 0, 5, ETO_CLIPPED, &rt, L"Привет", 6, NULL);
}
*/



int Contact::getIconIndex() const{
    if (composing) return icons::ICON_COMPOSING_INDEX;
    if (nUnread>0) return icons::ICON_MESSAGE_INDEX;
    int icon=(status==presence::OFFLINE)? offlineIcon: status;
	//int st=(status==Presence.PRESENCE_OFFLINE)?offline_type:status;
    if (icon<=presence::UNKNOWN) return icon+transpIndex; 
    return icon;
}


const wchar_t * Contact::getText() const{ return wjid.c_str(); }

const std::string Contact::getFullName() const{
    if (nickname.length()==0) return rosterJid;
    return nickname+" <"+rosterJid+">";
}

const std::string Contact::getName() const{
    if (nickname.length()==0) return rosterJid;
    return nickname;
}

const std::string Contact::getStatusMessage() const{
	if (statusMessage.length()==0) return "";
	return statusMessage;
}

const std::string Contact::getClientIdIcon() const{
	if (nodeInfo.length()==0) return "";
	return nodeInfo;
}


//////////////////////////////////////////////////////////////////////////
bool Contact::compareKST( Contact::ref left, Contact::ref right ) {
    Contact *l=left.get();
    Contact *r=right.get();
    if (l->sortKey < r->sortKey) return true;
    if (l->sortKey > r->sortKey) return false;
    if (l->status < r->status) return true;
    if (l->status > r->status) return false;
    return (_wcsicmp(left->getText(), right->getText()) < 0);
}
//////////////////////////////////////////////////////////////////////////
bool Contact::compareKT( Contact::ref left, Contact::ref right ) {
    Contact *l=left.get();
    Contact *r=right.get();
    if (l->sortKey < r->sortKey) return true;
    if (l->sortKey > r->sortKey) return false;
    return (_wcsicmp(left->getText(), right->getText()) < 0);
}
void Contact::setmood_icon(int i){
moodicon=i;
 update();
}
void Contact::settuneon(){tuneicon=1; update();}
void Contact::settuneoff(){tuneicon=0; update();}
void Contact::update() {
    std::string s=(nickname.empty())? jid.getBareJid():nickname;
    std::string resource=jid.getResource();
	clientIcon=0;	if(Config::getInstance()->confclient){/*
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"bombus-im.org/ng")!=NULL){clientIcon=icons::ICON_BOMBUS_NG;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"tkabber")!=NULL){clientIcon=icons::ICON_TKAB;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"bombusmod-qd")!=NULL){clientIcon=icons::ICON_BOMBUS_QD;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"gajim")!=NULL){clientIcon=icons::ICON_GAJIM;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"Siemens Native Jabber Client")!=NULL){clientIcon=icons::ICON_SJC;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"qip")!=NULL){clientIcon=icons::ICON_QIP;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"pidgin")!=NULL){clientIcon=icons::ICON_PIDGIN;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"miranda")!=NULL){clientIcon=icons::ICON_MIRANDA;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"kopete")!=NULL){clientIcon=icons::ICON_KOPET;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"bombus-im.org/java")!=NULL){clientIcon=icons::ICON_BOMBUS;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"psi")!=NULL){clientIcon=icons::ICON_PSI;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"bombusmod.net.ru")!=NULL){clientIcon=icons::ICON_BOMBUSMOD;}else{
	if(wcsstr(utf8::utf8_wchar(getClientIdIcon()).c_str(),L"bombusng-qd.googlecode.com")!=NULL){clientIcon=icons::ICON_BOMBUS_QD_NG;

	}else{
		clientIcon=0;

	}}}}}}}}}}}}}}*/
Skin * il= dynamic_cast<Skin *>(skin.get());
 std::string ClientI=getClientIdIcon();

 if(ClientI.length()>2 ){if (il) clientIcon=il->getKlientIndex((char*)ClientI.c_str());}else clientIcon=0;
	}
    if (resource.length()) { s+='/'; s+=resource; }
    wjid=utf8::utf8_wchar( s );
    init();
}

void Contact::messageDelivered(const std::string & id){
    for (size_t index=0; index<messageList->size(); index++) {
        Message::ref msg=boost::dynamic_pointer_cast<Message>(messageList->operator [](index));
        if (!msg) continue;
        if (msg->type!=Message::SENT) continue;
        if (id!=msg->id) continue;
        msg->delivered=true;
    }    
}


void Contact::processPresence( JabberDataBlockRef block ) {

    std::string type=block->getAttribute("type");
    std::string priority=block->getChildText("priority");
    std::string status=block->getChildText("status");

    presence::PresenceIndex typeIndex=presence::OFFLINE;
    presence::PresenceIndex type2=presence::NOCHANGE; //no change
    Message::MsgType msgType=Message::PRESENCE;

    if (type=="unavailable") { 
        typeIndex=presence::OFFLINE;
        type="offline";
    } else if (type=="subscribe") { 
        msgType=Message::PRESENCE_ASK_SUBSCR;
        //TODO:
    } else if (type=="subscribed") {
        msgType=Message::PRESENCE_SUBSCRIBED;
        //TODO:
    } else if (type=="unsubscribe") {
        //TODO:
    } else if (type=="unsubscribed") {
        msgType=Message::PRESENCE_UNSUBSCRIBED;
        //TODO:
    } else if (type=="error") {
        typeIndex=presence::OFFLINE;
        type2=presence::PRESENCE_ERROR;
        //todo: extract error text here
    } else {
        type=block->getChildText("show");
        if (type=="chat") typeIndex=presence::CHAT; else
            if (type=="away") typeIndex=presence::AWAY; else
                if (type=="xa") typeIndex=presence::XA; else
                    if (type=="dnd") typeIndex=presence::DND; else {
                        typeIndex=presence::ONLINE;
                        type="online";
                    }
    }
/*
<presence id="36">
<priority>30</priority>
<show>xa</show>
<x xmlns="vcard-temp:x:update" />
<c xmlns="http://jabber.org/protocol/caps" node="http://gajim.org" ver="2sSROk7c/BXz4QRPKkeriaW4V6M=" hash="sha-1" />icons::ICON_BOMBUS_NG
<status>ага</status>
</presence>
*/
typevs=type;

    this->status=typeIndex;
    if (type2!=presence::NOCHANGE) this->offlineIcon=type2;

    if (nickname.empty() && this->status<=presence::DND) {
        JabberDataBlockRef nick=block->findChildNamespace("nick","http://jabber.org/protocol/nick");
        if (nick) {
            nickname=nick->getText();
            std::trim(nickname);
		}
    }
	
	JabberDataBlockRef nodeBlock=block->findChildNamespace("c","http://jabber.org/protocol/caps");
	   if(nodeBlock){
          nodeInfo = nodeBlock->getAttribute("node");
		  //Log::getInstance()->msg("[***]Get node CLIENT: " + block->getAttribute("from") + " " + nodeInfo 
		  //	  + " VERSION: " + nodeBlock->getAttribute("ver"));
	   }else{nodeInfo = "";}
	
    update();

    std::string body=type;
    if (status.length()) {
        body+=" (";
        body+=status;
		statusMessage = status;
        body+=')';
    }
statusMessage2=statusMessage;
    if (priority.length()) {
        body+=" [";
        //std::strAppendInt(body, priority);
        body+=priority;
        body+=']';
    }

    Message::ref msg=Message::ref(new Message(body, block->getAttribute("from"), false, msgType, Message::extractXDelay(block) ));

    if (messageList->size()==1) {
        //verify if it is presence;
        Message::ref mfirst=boost::dynamic_pointer_cast<Message>(messageList->front());
        if (mfirst) if (mfirst->type==Message::PRESENCE)
            this->messageList->erase( this->messageList->begin());
    }
	if (Config::getInstance()->showStatusInSimpleChat==false){
        //Отображение статусной информации у simple контакта
	    if(msgType!=Message::PRESENCE){
          this->messageList->push_back(msg); //исключаем презенсы
	    } 
	}else{
      this->messageList->push_back(msg);
	}

}
