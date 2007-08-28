/*
 * Created on Jun 14, 2007
 *
 */
package edu.fhm.mmixmb.comm;

import java.io.*;
import java.util.*;

public class Message {
	
	public static final byte TYPE_BUS = (byte)(0x80);
	public static final byte TYPE_TIME = (byte)(0x40);
	public static final byte TYPE_ADDRESS = (byte)(0x20);
	public static final byte TYPE_ROUTE = (byte)(0x10);
	public static final byte TYPE_PAYLOAD = (byte)(0x08);
	public static final byte TYPE_REQUEST = (byte)(0x04);
	public static final byte TYPE_LOCK = (byte)(0x02);
	
	public static final byte ID_REGISTER = (byte)(0xFA);
	public static final byte ID_UNREGISTER = (byte)(0xFB);
	public static final byte ID_INTERRUPT = (byte)(0xFC);
	public static final byte ID_RESET = (byte)(0xFD);
	public static final byte ID_POWEROFF = (byte)(0xFE);
	public static final byte ID_POWERON = (byte)(0xFF);

	private byte type;
	private byte size;
	private byte slot;
	private byte id;
	private int timeStamp = 0;
	private long address = 0;
	private ArrayList<Byte> payload = new ArrayList<Byte>(); 

	public Message(MotherboardConnection connection) throws IOException {
		readHeader(connection);
		if(hasTimeStamp()){
			readTimeStamp(connection);
		}
		if(hasAddress()){
			readAddress(connection);
		}
		if(hasPayload()){
			readPayload(connection);
		}
	}
	
	public Message(){
		
	}

	private void readPayload(MotherboardConnection connection) throws IOException {
		for(int i = 0; i < payloadSize(); i++){
			payload.add(connection.readByte());
		}
	}

	private void readAddress(MotherboardConnection connection) throws IOException {
		address = 0;
		for(int i = 0; i < 8; i++){
			address = (address << 8) + connection.readByte(); 
		}
	}

	private void readTimeStamp(MotherboardConnection connection) throws IOException {
		timeStamp = 0;
		for(int i = 0; i < 4; i++){
			timeStamp = (timeStamp << 8) + connection.readByte(); 
		}
	}

	private void readHeader(MotherboardConnection connection) throws IOException {
		type = connection.readByte();
		size = connection.readByte();
		slot = connection.readByte();
		id   = connection.readByte();
	}
	
	public boolean isBusMessage(){
		return (type & TYPE_BUS) != 0;
	}
	
	public boolean hasTimeStamp(){
		return (type & TYPE_TIME) != 0;
	}
	
	public boolean hasAddress(){
		return (type & TYPE_ADDRESS) != 0;
	}
	
	public boolean hasRoute(){
		return (type & TYPE_ROUTE) != 0;
	}
	
	public boolean hasPayload(){
		return (type & TYPE_PAYLOAD) != 0;
	}
	
	public void setType(byte type){
		this.type = type;
	}
	
	public boolean isRequest(){
		return (type & 0x04) != 0;
	}
	
	public boolean hasLock(){
		return (type & 0x02) != 0;
	}

	/**
	 * @return the address
	 */
	public long getAddress() {
		return address;
	}

	public byte getId() {
		return id;
	}

	public byte getSize() {
		return size;
	}

	public byte getSlot() {
		return slot;
	}

	public int getTimeStamp() {
		return timeStamp;
	}
	
	public byte[] toByteArray(){
		int arraySize = 4;
		if(hasTimeStamp()){
			arraySize += 4;
		}
		if(hasAddress()){
			arraySize += 8;
		}
		if(hasPayload()){
			arraySize += payloadSize();
		}
		ArrayList<Byte> message = new ArrayList<Byte>();
		message.add(type);
		message.add(size);
		message.add(slot);
		message.add(id);
		if(hasTimeStamp()){
			append(timeStamp, message);
		}
		if(hasAddress()){
			append(address, message);
		}
		if(hasPayload()){
				append(payload, message);
		}
		byte[] m = new byte[message.size()];
		int i = 0;
		for(byte b : message){
			m[i++] = b;
		}
		return m;
	}
	
	public int payloadSize(){
		return (size + 1) * 8;
	}

	public static void append(int value, ArrayList<Byte> a) {
		for(int i = 3; i>= 0; i--){
			a.add((byte)((value >>> 8*i) & 0xFF));
			value >>>= 8;
		}
	}
	
	public static void append(long value, ArrayList<Byte> a) {
		for(int i = 7; i >= 0; i--){
			a.add((byte)((value >>> 8*i) & 0xFF));
		}
	}
	
	public static void append(ArrayList<Byte> value, ArrayList<Byte> a) {
		for(int i = 0; i < value.size(); i++){
			a.add(value.get(i));
		}
	}
	
	public void appendToPayload(long l) {
		append(l, payload);
	}
	
	public void appendToPayload(int i) {
		append(i, payload);
	}
	
	public void appendToPayload(ArrayList<Byte> list) {
		append(list, payload);
	}
	
	public void appendToPayload(String s) {
		for(int i = 0; i < s.length(); i++){
			payload.add((byte)(s.charAt(i) & 0xFF));
		}
	}

	public void setId(byte id) {
		this.id = id;
	}

	public void setSize(byte size) {
		this.size = size;
	}

	public void setSlot(byte slot) {
		this.slot = slot;
	}

	public void setAutoSize() {
		size = (byte)(payload.size());
	}

	public void finalizePayloadAndSize() {
		while((payload.size() % 8) != 0){
			payload.add((byte)0);
		}
		size = (byte)(payload.size() / 8 - 1);
	}
	
}
