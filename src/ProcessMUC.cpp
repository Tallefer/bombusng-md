
#include "ProcessMUC.h"
#include "utf8.hpp"

#include "TabCtrl.h"
#include "ChatView.h"

#include "JabberStream.h"

#include "Image.h"
#include "config.h"//���������� ������

extern TabsCtrlRef tabs;
extern RosterListView::ref rosterWnd;
extern ImgListRef skin;


//////////////////////////////////////////////////////////////////////////
MucContact::ref getMucContactEntry(const std::string &jid, ResourceContextRef rc) {
    MucContact::ref c=boost::dynamic_pointer_cast<MucContact> (rc->roster->findContact(jid));
    if (!c) {
        c=MucContact::ref(new MucContact(jid));
        rc->roster->addContact(c);
    }
    return c;
}

enum MPA {
    NONE=0,
    ENTER=1,
    LEAVE=2
};

char * roleName[]= { "none", "visitor", "participant", "moderator" };
char * affiliationName[]= { "outcast", "none", "member", "admin", "owner" };

MucContact::Role getRoleIndex(const std::string &role) {
    if (role=="visitor") return MucContact::VISITOR;
    if (role=="participant") return MucContact::PARTICIPANT;
    if (role=="moderator") return MucContact::MODERATOR;
    return MucContact::NONE_ROLE;
}
MucContact::Affiliation getAffiliationIndex(const std::string &role) {
    if (role=="outcast") return MucContact::OUTCAST;
    if (role=="member") return MucContact::MEMBER;
    if (role=="admin") return MucContact::ADMIN;
    if (role=="owner") return MucContact::OWNER;
    return MucContact::NONE;
}
//////////////////////////////////////////////////////////////////////////
ProcessResult ProcessMuc::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef xmuc=block->findChildNamespace("x", "http://jabber.org/protocol/muc#user");
    if (!xmuc) return BLOCK_REJECTED;

    const std::string &from=block->getAttribute("from");
    const std::string &type=block->getAttribute("type");

    Jid roomNode;
    roomNode.setJid(from);

    std::string message;

    //1. group
    MucGroup::ref roomGrp;
    roomGrp=boost::dynamic_pointer_cast<MucGroup> (rc->roster->findGroup(roomNode.getBareJid()));

    if (!roomGrp) return BLOCK_PROCESSED; //dropped presence

    MucContact::ref c=getMucContactEntry(from, rc);

    MPA action=NONE;

    if (type=="error") {
        JabberDataBlockRef error=block->getChildByName("error");
        int errCode=atoi(error->getAttribute("code").c_str());

        //todo: if (status>=Presence.PRESENCE_OFFLINE) testMeOffline();
        action=LEAVE;
        
        //todo: if (errCode!=409 || status>=Presence.PRESENCE_OFFLINE)  setStatus(presenceType);

        std::string errText=error->getChildText("text");
        if (errText.length()>0) message=errText; // if error description is provided by server
        else // legacy codes
            switch (errCode) {
                case 401: message=utf8::wchar_utf8(L"������������ ������");
                case 403: message=utf8::wchar_utf8(L"�� �������� � ���� �������");
                case 404: message=utf8::wchar_utf8(L"������� �� ���������");
                case 405: message=utf8::wchar_utf8(L"�� �� ������ ��������� ������� �� ���� �������");
                case 406: message=utf8::wchar_utf8(L"Reserved roomnick must be used");
                case 407: message=utf8::wchar_utf8(L"������� ���� ��� ����������");
                case 409: message=utf8::wchar_utf8(L"��� ��� ����� ������ ");
                case 503: message=utf8::wchar_utf8(L"������������ ����� ������������� ���� ���������� � ���� �������");
                default: message=*(error->toXML());
            }
    } else {
        JabberDataBlockRef item=xmuc->getChildByName("item");   

        MucContact::Role role = 
            getRoleIndex(item->getAttribute("role"));
        c->sortKey=MucContact::MODERATOR-role;
        
        MucContact::Affiliation affiliation = 
            getAffiliationIndex(item->getAttribute("affiliation"));

        boolean roleChanged= c->role != role;
        boolean affiliationChanged= c->affiliation !=affiliation;
    
        c->role=role;
        c->affiliation=affiliation;

        

        //setSortKey(nick);

        switch (role) {
        case MucContact::MODERATOR:
            c->transpIndex=icons::ICON_MODERATOR_INDEX;
            break;
        case MucContact::VISITOR:
            {
                Skin * il= dynamic_cast<Skin *>(skin.get());
                c->transpIndex=(il)? il->getBaseIndex("visitors") : 0;
                break;
            }
        default:
            c->transpIndex=0;
        }

        JabberDataBlockRef statusBlock=xmuc->getChildByName("status");
        int statusCode=(statusBlock)? atoi(statusBlock->getAttribute("code").c_str()) : 0; 

        message=c->jid.getResource(); // nick

        if (type=="unavailable") {
            action=LEAVE;
            std::string reason=item->getChildText("reason");

            switch (statusCode) {
            case 303:
                message+=utf8::wchar_utf8(L" ������ �������� ��� ");
                message+=item->getAttribute("nick");
                c->jid.setResource(item->getAttribute("nick"));
                c->rosterJid=c->jid.getJid(); //for vCard
                c->update();
                action=NONE;
                break;

            case 307: //kick
            case 301: //ban
                message+=(statusCode==307)?utf8::wchar_utf8(L" ������ ") : utf8::wchar_utf8(L" ������� ");
                message+="(";
                message+=reason;
                message+=")";

                if (c->realJid.length()>0){
                    message+=" - ";
                    message+=c->realJid;
                }
                break;

            case 321:
                message+=utf8::wchar_utf8(L" �� ����� � ������� ������ ��� ����������");
                break;

            case 322:
                message+=utf8::wchar_utf8(L" ������� ����� ������ ��� ����������-��� ������� ");
                break;

            default:
                {
                    message+=utf8::wchar_utf8(L" ����� �� ������� ");
                    const std::string & status=block->getChildText("status");
                    if (status.length()) {
                        message+=" (";
                        message+=status;
                        message+=")";
                    }
                }
            }
        } else { //onlines
            action=ENTER;
            if (c->status>=presence::OFFLINE) {
                // first online
                std::string realJid=item->getAttribute("jid");
                if (realJid.length()) {
                    c->realJid=realJid;
                    message+=" (";
                    message+=realJid;  //for moderating purposes
                    message+=")";
                }
                message+=utf8::wchar_utf8(L" ����� � ������� ");
                message+=roleName[role];

                if (affiliation!=MucContact::NONE) {
                    message+=" - ";
                    message+=affiliationName[affiliation-MucContact::OUTCAST];

                    const std::string & status=block->getChildText("status");
                    if (status.length()) {
                        message+=" (";
                        message+=status;
                        message+=")";
                    }
                }
			} else {
                //change status
				message+=utf8::wchar_utf8(L" ������ ������: ");

                if ( roleChanged ) message+=roleName[role];
                if (affiliationChanged) {
                    if (roleChanged) message+=utf8::wchar_utf8(L" � ");
                    message+=(affiliation==MucContact::NONE)? 
                        utf8::wchar_utf8(L"��������") : affiliationName[affiliation-MucContact::OUTCAST];
                }
                if (!roleChanged && !affiliationChanged) {
                    const std::string &show=block->getChildText("show");
                    if (show.length()==0) message+="online";
                    else message+=show;

                    const std::string & status=block->getChildText("status");
                    if (status.length()) {
                        message+=" (";
                        message+=status;
                        message+=")";
                    }

                }
            }
        }
    }

    if (c.get()==roomGrp->selfContact.get()) {
        switch (action) {
            case ENTER:
                // room contact is online
                roomGrp->room->status=presence::ONLINE;
                break;
            case LEAVE:
                // room contact is offline
                roomGrp->room->status=presence::OFFLINE;
                // make all occupants offline
                rc->roster->setStatusByFilter(roomNode.getBareJid(), presence::OFFLINE);
                break;
        }
    }

    {
        ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(c).get());

        bool ascroll=(cv==NULL)? false: cv->autoScroll();

        c->processPresence(block);

        if (ascroll) {
            cv->moveEnd();
        }
        if (cv) if (IsWindowVisible(cv->getHWnd())) cv->redraw();
    }
    rc->roster->makeViewList();


    {
        Message::ref msg=Message::ref(new Message(message, from, false, Message::PRESENCE, Message::extractXDelay(block) ));
        Contact::ref room=roomGrp->room;
        ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(room).get());
        bool ascroll=(cv==NULL)? false: cv->autoScroll();
		if (Config::getInstance()->showMucPresences==false){
	    //����� ���������� ������� ����������� ���������
		  if (msg->type!=Message::PRESENCE){ 
			 //���� �������� ��������� - ������-�������
             room->messageList->push_back(msg);
			 //��������� ��������� � ���
		  }		
		}else{
             room->messageList->push_back(msg);
			 //��������� ���������,������� ��������� � ���		
		}
		if (ascroll) {
            cv->moveEnd();
        }
        if (cv) if (IsWindowVisible(cv->getHWnd())) cv->redraw();
    }

    return BLOCK_PROCESSED;
}


void ProcessMuc::initMuc( const std::string &jid, const std::string &password, ResourceContextRef rc) {
    Jid roomNode;
    roomNode.setJid(jid);

    //1. group
    MucGroup::ref roomGrp;
    roomGrp=boost::dynamic_pointer_cast<MucGroup> (rc->roster->findGroup(roomNode.getBareJid()));
    if (!roomGrp) {
        roomGrp=MucGroup::ref(new MucGroup(roomNode.getBareJid(), roomNode.getUserName()));
        rc->roster->addGroup(roomGrp);
    }
    roomGrp->password=password;

    //2. room contact
    MucRoom::ref room=roomGrp->room;
    if (!room) {
        room=MucRoom::ref(new MucRoom(jid));
        roomGrp->room=room;
        room->group=roomGrp->getName();
    }

    rosterWnd->openChat(room);
    

    //3. selfcontact
    roomGrp->selfContact=getMucContactEntry(jid, rc);
}

//////////////////////////////////////////////////////////////////////////

MucGroup::MucGroup( const std::string &jid, const std::string &name ) {
    groupName=jid;
    this->type=MUC;
    sortKey=utf8::utf8_wchar(name);
    wstr=sortKey;
    expanded=true;
    init();
}

void MucGroup::addContacts( ODRList *list ) {
    list->push_back(room); //there are no MucRoom contact in roster data, 
    //room messages will be count separately ;)
}


//////////////////////////////////////////////////////////////////////////
MucRoom::MucRoom( const std::string &jid ) {
    this->jid=Jid(jid);
    this->rosterJid=jid;
    //this->nickname=nickname;
    this->status=presence::OFFLINE;
    offlineIcon=presence::OFFLINE;

    nUnread=0;
    composing=false; acceptComposing=false;

    transpIndex=icons::ICON_GROUPCHAT_INDEX;

    update();
    messageList=ODRListRef(new ODRList);
}

void MucRoom::update() {

    wjid=utf8::utf8_wchar( jid.getBareJid() );

    init();
}
//////////////////////////////////////////////////////////////////////////
MucContact::MucContact( const std::string &jid ) 
{
    this->jid=Jid(jid);
    this->rosterJid=jid;

    this->group=this->jid.getBareJid();

    this->status=presence::OFFLINE;
    offlineIcon=presence::OFFLINE;

    composing=false;
    acceptComposing=false;

    nUnread=0;

    transpIndex=0;

    update();
    messageList=ODRListRef(new ODRList);
    enableServerHistory=Contact::DISABLED_STATE;
}

void MucContact::update() {clientIcon=0;if(Config::getInstance()->confclient){/*
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

 if(ClientI.length()>2){if (il) clientIcon=il->getKlientIndex((char*)ClientI.c_str());}else clientIcon=0;
}
    wjid=utf8::utf8_wchar( jid.getResource() );
    init();
}

void MucContact::changeRole( ResourceContextRef rc, Role newRole ) {
    BOOST_ASSERT(newRole<=MODERATOR);

    JabberDataBlockRef item=JabberDataBlockRef(new JabberDataBlock("item"));
    item->setAttribute("nick",jid.getResource());
    item->setAttribute("role",roleName[newRole]);
    //todo:    
    //  if (!reason.empty) item->addChild("reason", reason);

    changeMucItem(rc, item);
}

void MucContact::changeAffiliation( ResourceContextRef rc, Affiliation newAffiliation ) {
    BOOST_ASSERT(newAffiliation<=OWNER);

    JabberDataBlockRef item=JabberDataBlockRef(new JabberDataBlock("item"));
    Jid rj(realJid);
    item->setAttribute("jid",rj.getBareJid());
    item->setAttribute("affiliation",affiliationName[newAffiliation-OUTCAST]);
    //todo:    
    //  if (!reason.empty) item->addChild("reason", reason);

    changeMucItem(rc, item);
}

void MucContact::changeMucItem(ResourceContextRef rc, JabberDataBlockRef item) {

    JabberDataBlock iq("iq");
    iq.setAttribute("type","set");
    iq.setAttribute("id","muc-a");
    iq.setAttribute("to",jid.getBareJid());

    iq.addChildNS("query","http://jabber.org/protocol/muc#admin")
        ->addChild(item);
    rc->jabberStream->sendStanza(iq);
}