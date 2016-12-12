package eu.ewall.sensing.socket;

public class Socket {
	private String room;
	private String name;
	private String type;
	private double powerNow;
	private double dailyEnergy;
	private boolean socketOn;
	
	public Socket(String room, String type, String name) {
		this.room = room.toLowerCase();
		this.name = name;
		this.type = type;
		this.socketOn = true;
		powerNow = 0;
		dailyEnergy = 0;
	}
	
	public String getRoom() {
		return room;
	}
	public void setRoom(String room) {
		this.room = room.toUpperCase();
	}
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	public String getType() {
		return type;
	}
	public void setType(String type) {
		this.type = type;
	}
	public double getPowerNow() {
		return powerNow;
	}
	public void setPowerNow(double power) {
		this.powerNow = power;
	}
	public double getDailyEnergy() {
		return dailyEnergy;
	}
	public void setDailyEnergy(double dailyEnergy) {
		this.dailyEnergy = dailyEnergy;
	}
	public boolean getSocketOn() {
		return socketOn;
	}
	public void setSocketOn(boolean socketOn) {
		this.socketOn = socketOn;
	}
}
