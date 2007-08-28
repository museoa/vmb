/*
 * Created on Jun 14, 2007
 *
 */
package net.sourceforge.vmb.comm;

public class MessageFactory {

	public static Message createRegistrationMessage(long address, long limit, long mask, String name){
		Message message = new Message();
		message.setType((byte)(Message.TYPE_BUS | Message.TYPE_PAYLOAD));
		message.appendToPayload(address);
		message.appendToPayload(limit);
		message.appendToPayload(mask);
		message.appendToPayload(name);
		message.finalizePayloadAndSize();
		message.setId(Message.ID_REGISTER);
		return message;
	}

}
