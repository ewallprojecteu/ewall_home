package eu.ewall.sensing.socket;

import java.util.Map;

import org.ektorp.CouchDbConnector;
import org.ektorp.CouchDbInstance;
import org.ektorp.http.HttpClient;
import org.ektorp.http.StdHttpClient;
import org.ektorp.impl.StdCouchDbConnector;
import org.ektorp.impl.StdCouchDbInstance;

public class CouchDbConnection {
	private CouchDbConnector db = null;

	public CouchDbConnection(String couchdburl, String database, Boolean resume) {
		HttpClient httpClient = null;
		CouchDbInstance dbInstance = null;		
		try {
			httpClient = new StdHttpClient.Builder().url(couchdburl).build();
			dbInstance = new StdCouchDbInstance(httpClient);
			if (dbInstance.checkIfDbExists(database) == false) {
				dbInstance.createDatabase(database);
			}
			if (!resume & dbInstance.checkIfDbExists(database)) {
				dbInstance.deleteDatabase(database);
				dbInstance.createDatabase(database);
			}
			db = new StdCouchDbConnector(database, dbInstance);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void writeData(Map<String, Object> data) throws Exception {
		if (db != null && data !=null)
			db.create(data);
	}

}
