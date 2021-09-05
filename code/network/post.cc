// post.cc 
// 	Routines to deliver incoming network messages to the correct
//	"address" -- a mailbox, or a holding area for incoming messages.
//	This module operates just like the US postal service (in other
//	words, it works, but it's slow, and you can't really be sure if
//	your mail really got through!).
//
//	Note that once we prepend the MailHdr to the outgoing message data,
//	the combination (MailHdr plus data) looks like "data" to the Network 
//	device.
//
// 	The implementation synchronizes incoming messages with threads
//	waiting for those messages.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "post.h"
#include "system.h"

#include <strings.h> /* for bzero */

//----------------------------------------------------------------------
// Mail::Mail
//      Initialize a single mail message, by concatenating the headers to
//	the data.
//
//	"pktH" -- source, destination machine ID's
//	"mailH" -- source, destination mailbox ID's
//	"data" -- payload data
//----------------------------------------------------------------------

Mail::Mail(PacketHeader pktH, MailHeader mailH, char *msgData)
{
    // if the incoming data is NULL then this will be a dummy mail that just contains packet and mail header 
    ASSERT(mailH.length <= MaxMailSize);

    pktHdr = pktH;
    mailHdr = mailH;
    numTrials = 0;
    if(msgData != NULL)
        bcopy(msgData, data, mailHdr.length);  // There is a problem here when msgData is NULL
    // DEBUG('n', "A mail is created successfully\n");
    
}

//----------------------------------------------------------------------
// MailBox::MailBox
//      Initialize a single mail box within the post office, so that it
//	can receive incoming messages.
//
//	Just initialize a list of messages, representing the mailbox.
//----------------------------------------------------------------------


MailBox::MailBox()
{ 
    messages = new SynchList(); 
}

//----------------------------------------------------------------------
// MailBox::~MailBox
//      De-allocate a single mail box within the post office.
//
//	Just delete the mailbox, and throw away all the queued messages 
//	in the mailbox.
//----------------------------------------------------------------------

MailBox::~MailBox()
{ 
    delete messages; 
}

//----------------------------------------------------------------------
// PrintHeader
// 	Print the message header -- the destination machine ID and mailbox
//	#, source machine ID and mailbox #, and message length.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//----------------------------------------------------------------------

static void 
PrintHeader(PacketHeader pktHdr, MailHeader mailHdr)
{
    printf("From (%d, %d) to (%d, %d) bytes %d\n",
    	    pktHdr.from, mailHdr.from, pktHdr.to, mailHdr.to, mailHdr.length);
}

//----------------------------------------------------------------------
// MailBox::Put
// 	Add a message to the mailbox.  If anyone is waiting for message
//	arrival, wake them up!
//
//	We need to reconstruct the Mail message (by concatenating the headers
//	to the data), to simplify queueing the message on the SynchList.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

void 
MailBox::Put(PacketHeader pktHdr, MailHeader mailHdr, char *data)
{ 
    Mail *mail = new Mail(pktHdr, mailHdr, data); 

    messages->Append((void *)mail);	// put on the end of the list of 
					// arrived messages, and wake up 
					// any waiters
}

//----------------------------------------------------------------------
// MailBox::Get
// 	Get a message from a mailbox, parsing it into the packet header,
//	mailbox header, and data. 
//
//	The calling thread waits if there are no messages in the mailbox.
//
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

void 
MailBox::Get(PacketHeader *pktHdr, MailHeader *mailHdr, char *data) 
{ 
    /*  THIS IS CLOSED FOR RESOLVING MERGE CONFLICTS **** 
    // we should check here if the msg still exists in history or not.
    Mail *mail = new Mail(*pktHdr, *mailHdr, NULL);
    if (!postOffice->history->IsEmpty()){
        DEBUG('n', "We did not get an ACK yet\n");
        postOffice->notReceived-> P();
        mail = (Mail *) postOffice->history->Remove();
    }
    else{
        mail = (Mail *) messages->Remove();   // remove message from list;
                    // will wait if list is empty
    }

    DEBUG('n', "Waiting for mail in mailbox\n");
    

    *pktHdr = mail->pktHdr;
    *mailHdr = mail->mailHdr;
    if (DebugIsEnabled('n')) {
	printf("Got mail from mailbox: ");
	PrintHeader(*pktHdr, *mailHdr);
    }
    bcopy(mail->data, data, mail->mailHdr.length);
					// copy the message data into
					// the caller's buffer
    delete mail;			// we've copied out the stuff we
					// need, we can now discard the message
    DEBUG('n', "mail was deleted successfully\n");*/

    DEBUG('n', "Waiting for mail in mailbox\n");
    Mail *mail = (Mail *) messages->Remove();	// remove message from list;
						// will wait if list is empty

    *pktHdr = mail->pktHdr;
    *mailHdr = mail->mailHdr;
    if (DebugIsEnabled('n')) {
	printf("Got mail from mailbox: ");
	PrintHeader(*pktHdr, *mailHdr);
    }
    bcopy(mail->data, data, mail->mailHdr.length);
					// copy the message data into
					// the caller's buffer
    delete mail;			// we've copied out the stuff we
					// need, we can now discard the message

}

//----------------------------------------------------------------------
// PostalHelper, ReadAvail, WriteDone
// 	Dummy functions because C++ can't indirectly invoke member functions
//	The first is forked as part of the "postal worker thread; the
//	later two are called by the network interrupt handler.
//
//	"arg" -- pointer to the Post Office managing the Network
//----------------------------------------------------------------------

static void PostalHelper(int arg)
{ PostOffice* po = (PostOffice *) arg; po->PostalDelivery(); }
static void ReadAvail(int arg)
{ PostOffice* po = (PostOffice *) arg; po->IncomingPacket(); }
static void WriteDone(int arg)
{ PostOffice* po = (PostOffice *) arg; po->PacketSent(); }

//----------------------------------------------------------------------
// PostOffice::PostOffice
// 	Initialize a post office as a collection of mailboxes.
//	Also initialize the network device, to allow post offices
//	on different machines to deliver messages to one another.
//
//      We use a separate thread "the postal worker" to wait for messages 
//	to arrive, and deliver them to the correct mailbox.  Note that
//	delivering messages to the mailboxes can't be done directly
//	by the interrupt handlers, because it requires a Lock.
//
//	"addr" is this machine's network ID 
//	"reliability" is the probability that a network packet will
//	  be delivered (e.g., reliability = 1 means the network never
//	  drops any packets; reliability = 0 means the network never
//	  delivers any packets)
//	"nBoxes" is the number of mail boxes in this Post Office
//----------------------------------------------------------------------

PostOffice::PostOffice(NetworkAddress addr, double reliability, int nBoxes)
{
// First, initialize the synchronization with the interrupt handlers
    messageAvailable = new Semaphore("message available", 0);
    messageSent = new Semaphore("message sent", 0);
    sendLock = new Lock("message send lock");
    notReceived = new Semaphore("ack is not received yet", 0);

// Second, initialize the mailboxes
    netAddr = addr; 
    numBoxes = nBoxes;
    boxes = new MailBox[nBoxes];

// Third, initialize the network; tell it which interrupt handlers to call
    network = new Network(addr, reliability, ReadAvail, WriteDone, (int) this);

// Add the history list 
    history = new SynchList(); 
// Finally, create a thread whose sole job is to wait for incoming messages,
//   and put them in the right mailbox. 
    Thread *t = new Thread("postal worker");

    t->Fork(PostalHelper, (int) this);
}

//----------------------------------------------------------------------
// PostOffice::~PostOffice
// 	De-allocate the post office data structures.
//----------------------------------------------------------------------

PostOffice::~PostOffice()
{
    delete network;
    delete [] boxes;
    delete messageAvailable;
    delete messageSent;
    delete sendLock;
    delete history;
    delete notReceived;
}

//----------------------------------------------------------------------
// PostOffice::PostalDelivery
// 	Wait for incoming messages, and put them in the right mailbox.
//
//      Incoming messages have had the PacketHeader stripped off,
//	but the MailHeader is still tacked on the front of the data.
//----------------------------------------------------------------------

void
PostOffice::PostalDelivery()
{
    PacketHeader pktHdr;
    MailHeader mailHdr;
    char *buffer = new char[MaxPacketSize];

    for (;;) {
        // first, wait for a message
        messageAvailable->P();	
        pktHdr = network->Receive(buffer);

        mailHdr = *(MailHeader *)buffer;
        if (DebugIsEnabled('n')) {
	    printf("Putting mail into mailbox: ");
	    PrintHeader(pktHdr, mailHdr);
        }

	// check that arriving message is legal!
	ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);
	ASSERT(mailHdr.length <= MaxMailSize);

	// put into mailbox
        boxes[mailHdr.to].Put(pktHdr, mailHdr, buffer + sizeof(MailHeader));
    }
}

//----------------------------------------------------------------------
// PostOffice::Send
// 	Concatenate the MailHeader to the front of the data, and pass 
//	the result to the Network for delivery to the destination machine.
//
//	Note that the MailHeader + data looks just like normal payload
//	data to the Network.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

void
PostOffice::Send(PacketHeader pktHdr, MailHeader mailHdr, const char* data)
{
    char* buffer = new char[MaxPacketSize];	// space to hold concatenated
						// mailHdr + data

    if (DebugIsEnabled('n')) {
	printf("Post send: ");
	PrintHeader(pktHdr, mailHdr);
    }
    ASSERT(mailHdr.length <= MaxMailSize);
    
    ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);
   //ASSERT();
    
    // fill in pktHdr, for the Network layer
    pktHdr.from = netAddr;
    pktHdr.length = mailHdr.length + sizeof(MailHeader);

    // concatenate MailHeader and data
    bcopy(&mailHdr, buffer, sizeof(MailHeader));
    bcopy(data, buffer + sizeof(MailHeader), mailHdr.length);

    sendLock->Acquire();   		// only one message can be sent
					// to the network at any one time
    network->Send(pktHdr, buffer);
    messageSent->P();			// wait for interrupt to tell us
					// ok to send the next message
    sendLock->Release();

    delete [] buffer;			// we've sent the message, so
					// we can delete our buffer
}

//----------------------------------------------------------------------
// PostOffice::Send
// 	Retrieve a message from a specific box if one is available, 
//	otherwise wait for a message to arrive in the box.
//
//	Note that the MailHeader + data looks just like normal payload
//	data to the Network.
//
//
//	"box" -- mailbox ID in which to look for message
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

void
PostOffice::Receive(int box, PacketHeader *pktHdr, 
				MailHeader *mailHdr, char* data)
{
    ASSERT((box >= 0) && (box < numBoxes));

    boxes[box].Get(pktHdr, mailHdr, data);
    ASSERT(mailHdr->length <= MaxMailSize);
}


//----------------------------------------------------------------------
// TimeOutHandler()
//  Interrupt handler, called when a packet is sent and the sender did 
//  not receive an ACK back after time TEMPO. Here we have 2 cases 
//  either this scheduled handler will happen after the ACK is sent in 
//  the reply box or before so we will have to resend it again.

// This is a dummy function for accessing member variable reliableSend
//----------------------------------------------------------------------
static void TimeOutHandler(int arg)
{ 
    PostOffice *post_office = (PostOffice *) arg;

    //check if there is something in the history
    if(post_office->history->IsEmpty())
        // ACK arrived before the interrupt so we have nothing to do
        return;
    else{
        //get and resend this message from the history
        Mail *mail = (Mail *) post_office->history->GetItem();
        post_office->reliableSend(mail->pktHdr, mail->mailHdr, mail->data);
    }

}

//----------------------------------------------------------------------
// PostOffice::reliableSend
//  This is a wrapper method for Send() however, this version gurantee the 
//  reliablity through invoking an interrupt that will be handled by 
//  resending the message again
//
//  1. check if this message exists in history or not
//  2. check on the number of trials of this message 
//  3. send it 
//  4. schedule an interrupt after time TEMPO
//  5. wait for ACK 
//----------------------------------------------------------------------

void
PostOffice::reliableSend(PacketHeader pktHdr, MailHeader mailHdr, const char *data)
{


    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char buffer[MaxMailSize];
    // create a new mail object with headers
    // Let data be NULL because of conversion conflicts then re-assign it by bcopy
    
    Mail *mail = new Mail(pktHdr, mailHdr, NULL);
    //bcopy(mail->data, data, sizeof(mailHdr.length));
    //strcpy(data, mail->data);
    bcopy(data, mail->data, MaxMailSize);
    //PrintHeader(mail->pktHdr, mail->mailHdr);
    if(postOffice->history->IsEmpty()){
        // if yes, this will be the first time
        // to be sent so append it to history and
        // increment numTrials of this mail
        mail->numTrials++; // this should be 1 after execution
         //DEBUG('n', "we are about to send a msg\n");
        history->Append((void *)mail); 

    }
    else{
        // It is in history so get it and check numTrials
        //Mail *mail = (Mail *) history->GetItem();
        if(mail->numTrials < MAXREEMISSIONS){
            //increment trials and 
            mail->numTrials++;
            //send it again
            // Send(pktH, mailH, data);
        }
        else{
            // we exceeded the number of MAXREEMISSIONS so notify the thread
            printf("Ops!. You message could not be sent\n");
            return;
        }
    }
    DEBUG('n', "before normal send mailhdr_to is %d\n", mailHdr.to);
    postOffice->Send(pktHdr, mailHdr, data);
    //DEBUG('n', "MSG sent\n");
    //#ifndef INTERRUPT_H
    // Schedule the Interrupt
    interrupt->Schedule(TimeOutHandler, (int) this,TEMPO, NetworkSendInt);
    //DEBUG('n', "interrupt scheduled\n");
    //#endif
    //wait for ACK 
    inPktHdr.to = pktHdr.from;
    inMailHdr.to = mailHdr.from;
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    notReceived-> V();
    //DEBUG('n', "MSG is received\n");
}






//----------------------------------------------------------------------
// PostOffice::reliableReceive
//  This is a wrapper method for Receive() however, this version gurantee  
//  reliablity through invoking an interrupt that will be handled by 
//  resending the message again
//
//  1. Get a message from the box 
//  2. then send ACK to the sender using headrs of the packet and mail. 
//   
//  UPDATE : This design may conflict the process of sending an ACK because
//  we may find tha mail from Get() an ACK that a message has sent so we need
//  to figue out a way to remove it from the list.
//----------------------------------------------------------------------

void
PostOffice::reliableReceive(int box, PacketHeader *pktHdr, 
    MailHeader *mailHdr, char *data){
    // send an ACK is there is incoming message
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    const char *ack = "Got it!";

    //ASSERT((box >= 0) && (box < numBoxes));

    boxes[box].Get(pktHdr, mailHdr, data);

    //construct headers to send
    outPktHdr.to = pktHdr->from;
    outPktHdr.from = pktHdr->to;
    outMailHdr.to = mailHdr->from;
    outMailHdr.from = mailHdr->to;
    outMailHdr.length = strlen(ack) + 1;
    Send(outPktHdr, outMailHdr, ack);


}

//----------------------------------------------------------------------
// PostOffice::IncomingPacket
// 	Interrupt handler, called when a packet arrives from the network.
//
//	Signal the PostalDelivery routine that it is time to get to work!
//----------------------------------------------------------------------
void
PostOffice::IncomingPacket()
{ 
    messageAvailable->V(); 
}

//----------------------------------------------------------------------
// PostOffice::PacketSent
// 	Interrupt handler, called when the next packet can be put onto the 
//	network.
//
//	The name of this routine is a misnomer; if "reliability < 1",
//	the packet could have been dropped by the network, so it won't get
//	through.
//----------------------------------------------------------------------

void 
PostOffice::PacketSent()
{ 
    messageSent->V();
}

