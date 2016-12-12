package eu.ewall.platform.local.datamanager.couchdb;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.lightcouch.CouchDbClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import eu.ewall.platform.remoteproxy.api.RemoteProxyDBProvider;


/**
 * The Class CouchDbPullRequestManager.
 * 
 * @author emirmos
 */
public class CouchDbPullRequestManager implements RemoteProxyDBProvider {

	/** The logger. */
	private static Logger logger = LoggerFactory
			.getLogger(CouchDbPullRequestManager.class);

	/** The instance. */
	private static CouchDbPullRequestManager instance;

	/** The client. */
	private CouchDbClient client;

	/**
	 * Instantiates a new couch db pull request manager.
	 */
	private CouchDbPullRequestManager() {
	}

	/**
	 * Gets the single instance of CouchDbPullRequestManager.
	 *
	 * @return single instance of CouchDbPullRequestManager
	 */
	public static synchronized CouchDbPullRequestManager getInstance() {
		if (instance == null) {
			instance = new CouchDbPullRequestManager();
		}
		return instance;
	}

	/* (non-Javadoc)
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyDBProvider#intializeDBClient(org.apache.commons.configuration.PropertiesConfiguration)
	 */
	@Override
	public boolean intializeDBClient(PropertiesConfiguration properties) {

		try {
			String protocol = properties.getString("couchdb.protocol");
			String host = properties.getString("couchdb.host");
			String portString = properties.getString("couchdb.port");
			String username = properties.getString("couchdb.username");
			String password = properties.getString("couchdb.password");

			if (protocol == null) {
				protocol = "http";
			}
			if (host == null) {
				host = "127.0.0.1";
			}
			int port = 5984;
			if (portString != null) {
				port = Integer.parseInt(portString);
			}

			if (username != null) {
				if (username.length() == 0) {
					username = null;
				}
			}
			if (password != null) {
				if (password.length() == 0) {
					password = null;
				}
			}

			client = new CouchDbClient("version", false, protocol, host, port,
					username, password);

		} catch (NumberFormatException e) {
			logger.error("Error in intializing DB client: " + e.toString());
		}

		return true;

	}

	/* (non-Javadoc)
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyDBProvider#getVersion()
	 */
	@Override
	public String getVersion() {
		if (client != null) {
			try {
				JsonObject versionDoc = client
						.find(JsonObject.class, "version");
				JsonElement versionElement = versionDoc.get("version");
				String version = versionElement.getAsString();
				return version;
			} catch (Exception e) {
				logger.error("Error in retriving version information from local database: ", e);
			}
		} else {
			logger.warn("Couch DB client instance is null and not initialized. Unable to retrive eWall home version info.");
		}
		return null;
	}

	/* (non-Javadoc)
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyDBProvider#close()
	 */
	@Override
	public void close() {
		if (client != null) {
			logger.debug("Shuting down database connection manager used by RemoteProxyDBProvider.");
			client.shutdown();
		} else {
			logger.debug("Couch DB client instance is null and not initialized.");
		}

	}

}
