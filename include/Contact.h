#pragma once

#include <string>
#include "boostheaders.h"
#include <list>

#include "Message.h"

#include "IconTextElement.h"

#include "Presence.h"
#include "jid.h"

#include "VirtualListView.h"

class Contact : public IconTextElement {

protected:

public:
	ImageRef img_avatar; //Изображение-аватара контакта
    enum SH{
        DISABLED_STATE=-1,
        DEFAULT=0,
        ALLOW=1,
        BLOCK=2
    };

    typedef boost::shared_ptr<Contact> ref;
	Contact(
		const std::string &jid,
		const std::string &resource,
		const std::string &nickname,
		const std::string &statusMessage,
		const std::string &nodeInfo);
    Contact(){}

    Contact::ref clone();

    Jid jid;
    std::string nickname;
    std::string rosterJid;
    std::string group;
	std::string statusMessage;
	std::string nodeInfo;

    std::string subscr;

	
	std::string messtune;
	std::string messmood;
	std::string messact;
    void setmood_icon(int i);


    int sortKey;
    presence::PresenceIndex status;
    int offlineIcon;

    ODRListRef messageList;

    int enableServerHistory;

    bool composing;
    bool acceptComposing;

    //bool hasUnreadMsgs();

    int nUnread;

    static bool compareKST(Contact::ref left, Contact::ref right);
    static bool compareKT(Contact::ref left, Contact::ref right);

    const std::string getFullName() const;
    const std::string getName() const;
	const std::string getStatusMessage() const;
	const std::string getClientIdIcon() const;
	void settuneon();
	void settuneoff();
	std::string Tartist;
	std::string Ttitle;
	std::string Tsource;
	//virtual void settuneoff() ;
    virtual void processPresence(JabberDataBlockRef block);
    void messageDelivered(const std::string & id);
    //////////////////////////////////////////////////////////////////////////

    virtual int getColor() const;
	//virtual void draw(HDC hdc, RECT &rt) const;
    virtual int getIconIndex() const;
    virtual const wchar_t * getText() const;

    virtual void update();

    std::wstring wjid;
    int transpIndex;
private:
};

