package eu.ewall.sensing.socket;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SocketSensing {
	CouchDbConnection db;
	List<Socket> sockets;
	boolean changed;
	
	public SocketSensing(String couchdburl, String dbName, Boolean resume) {
		sockets = new ArrayList<Socket>();
		db = new CouchDbConnection(couchdburl, dbName, resume);
		changed = true;
	}

	public void put2db(String dateStr) {
		if (changed == true) {
			Map<String, Object> data = new HashMap<String, Object>();
			data.put("_id", dateStr);
			data.put("timestamp", dateStr);
			data.put("appliances", sockets);
			try {
				db.writeData(data);
				changed = false;
			} catch (Exception e) {
				System.out.println("Sockets - write to CouchDB failed: " + e.getMessage());					
			}
		}
	}

	public void addAppliance(String room, String type, String name) {
		Socket appliance = new Socket(room, type, name);
		sockets.add(appliance);
		changed = true;
	}

	public List<Socket> getSockets() {
		return sockets;
	}

	public void setSockets(List<Socket> sockets) {
		this.sockets = sockets;
	}

	public boolean isChanged() {
		return changed;
	}

	public void setChanged(boolean changed) {
		this.changed = changed;
	}

}