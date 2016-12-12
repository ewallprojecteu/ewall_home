package eu.ewall.platform.local.datamanager.couchdb;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import org.lightcouch.Changes;
import org.lightcouch.ChangesResult;
import org.lightcouch.CouchDbClient;
import org.slf4j.Logger;

import com.google.gson.JsonObject;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The listener interface for receiving RPCouchDb events. The class that is
 * interested in processing a RPCouchDb event implements this interface, and the
 * object created with that class is registered with a component using the
 * component's <code>addRPCouchDbListener<code> method. When
 * the RPCouchDb event occurs, that object's appropriate
 * method is invoked.
 *
 * @author emirmos
 */
public abstract class RPCouchDbListener extends Thread {

	/** The CouchDB connection heart beat. */
	protected final int heartBeat = 10000;

	/** The is running. */
	protected volatile boolean isRunning = true;

	/** The db client. */
	protected CouchDbClient dbClient;

	/** The rp rest client. */
	protected RPCloudClient rpCloudClient;

	/** The controller. */
	protected Controller controller;

	/** Corresponding device */
	protected Device device;

	/** The continue since. */
	protected ContinueSince continueSince;

	/** The latest seq. */
	protected String latestSeq;

	/** The logger. */
	protected Logger logger;

	/** The since begining. */
	private final String SINCE_BEGINING = "0";

	/**
	 * Instantiates a new RP couch db listener.
	 *
	 * @param dbClient
	 *            the db client
	 * @param rpCloudClient
	 *            the rp cloud client
	 * @param controller
	 *            the controller
	 * @param dbName
	 *            the db name
	 * @param dbType
	 *            the db type
	 * @param since
	 *            the since
	 */
	public RPCouchDbListener(CouchDbClient dbClient,
			RPCloudClient rpCloudClient, Controller controller, Device device, ContinueSince since) {
		this.rpCloudClient = rpCloudClient;
		this.dbClient = dbClient;
		this.device = device;
		this.continueSince = since;
		this.controller = controller;
	}

	/**
	 * Sync.
	 */
	public void sync() {
		/*
		 * Find last document from CouchDB (local) and compare with last
		 * document from MongoDB (cloud)
		 */
		long couchLastTimestamp = 0;
		String sinceChangesStart = SINCE_BEGINING;

		String sinceEnd = Integer.toString(Integer.parseInt(dbClient.context()
				.info().getUpdateSeq()) - 1);
		List<ChangesResult.Row> feedList = dbClient.changes().since(sinceEnd)
				.limit(1).getChanges().getResults();

		if (feedList.size() == 0) {
			logger.info("No local data for device "+device.getName());
			handleSyncingNotNeeded();
			return;
		}

		ChangesResult.Row lastRow = feedList.get(0);
		couchLastTimestamp = this.getTimestampForId(lastRow.getId());

		if (couchLastTimestamp == 0) {
			logger.info("Not able to parse local data timestamp for device "+device.getName());
			handleSyncingNotNeeded();
			return;
		}

		long mongoLastTimestamp = rpCloudClient.doGetLastTimestamp(
				device.getUuid().toString(), device.getType());

		/*
		 * If last documents not equal, go from firstly changed row in local
		 * database (found by iterating through rows sequences) and add missing
		 * documents to cloud using processEvent method
		 */
		if (mongoLastTimestamp >= couchLastTimestamp) {
			logger.info("Latest cloud data timestamp equal or newer than local data timestamp for device "+device.getName());
			handleSyncingNotNeeded();
			return;
		}

		Changes allChanges = dbClient.changes().since(SINCE_BEGINING)
				.continuousChanges();
		while (isRunning && allChanges.hasNext()) {
			ChangesResult.Row row = allChanges.next();
			if (row != null) {
				long couchTimestamp = this.getTimestampForId(row.getId());
				if (couchTimestamp > mongoLastTimestamp) {
					break;
				}
				sinceChangesStart = row.getSeq();
			}
		}
		allChanges.stop();

		List<ChangesResult.Row> feedChanges = dbClient.changes()
				.since(sinceChangesStart).includeDocs(true).getChanges()
				.getResults();
		for (ChangesResult.Row row : feedChanges) {
			this.processEvent(row.getDoc(), row.getSeq());
		}

		handleSynced();
	}

	/**
	 * Handle syncing not needed. Logging debug information and storing last
	 * sequence.
	 */
	private void handleSyncingNotNeeded() {
		logger.debug("Syncing db not needed for device: " + this.getDBName());
		this.storeLastSentUpdateSeq(dbClient.context().info().getUpdateSeq());
	}

	/**
	 * Handle synced. Logging debug information and storing last sequence.
	 */
	private void handleSynced() {
		logger.debug("Synced db for device: " + this.getDBName());
		this.storeLastSentUpdateSeq(dbClient.context().info().getUpdateSeq());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Thread#run()
	 */
	@Override
	public void run() {
		if (dbClient == null) {
			logger.error("dbClient not initialized.");
			return;
		}

		try {

			String since = this.getLastSentUpdateSeq();
			logger.debug(this.device.getName() + " database: starting listening since "
					+ since);

			Changes changes = dbClient.changes().since(since).includeDocs(true)
					.heartBeat(heartBeat).continuousChanges();

			latestSeq = since;
			while (isRunning && changes.hasNext()) {
				ChangesResult.Row feed = changes.next();
				if (feed != null) {

					final JsonObject feedObject = feed.getDoc();

					if (!this.processEvent(feedObject, feed.getSeq()))
						break;

				}

			}

			changes.stop();

		} catch (Exception e) {
			logger.error("ERROR in CouchDB connection: " + e.getMessage());
		}

		logger.info("Database " + this.getDBName() + " latest sequence: "
				+ latestSeq);
		this.storeLastSentUpdateSeq(latestSeq);
		dbClient.shutdown();
		logger.debug("Listening thread for " + this.getDBName()
				+ " database stopped.");

	}

	/**
	 * Process event.
	 *
	 * @param feedObject
	 *            the feed object
	 * @param seq
	 *            the seq
	 * @return true, if successful
	 */
	public abstract boolean processEvent(JsonObject feedObject, String seq);

	/**
	 * Gets the timestamp for id.
	 *
	 * @param id
	 *            the id
	 * @return the timestamp for id
	 */
	public long getTimestampForId(String id) {
		try {
			Date date = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSX")
					.parse(id);
			long timestamp = date.getTime();
			return timestamp;

		} catch (Exception e) {
			logger.warn(e.getMessage());
			return 0;
		}
	}

	/**
	 * Stop running.
	 */
	public void stopRunning() {
	}

	/**
	 * Gets the DB name.
	 *
	 * @return the DB name
	 */
	public String getDBName() {
		return device.getName();
	}

	/**
	 * Gets the db client.
	 *
	 * @return the db client
	 */
	public CouchDbClient getDbClient() {
		return dbClient;
	}

	/**
	 * Gets the last sent update seq.
	 *
	 * @return the last sent update seq
	 */
	public String getLastSentUpdateSeq() {
		// the default
		String sinceString = SINCE_BEGINING;

		if (continueSince == ContinueSince.FIRST)
			return SINCE_BEGINING;
		else if (continueSince == ContinueSince.LAST) {
			if (dbClient != null)
				return dbClient.context().info().getUpdateSeq();
			
			logger.warn(this.getDBName()
					+ " database listener getLastSentUpdateSeq(): dbClient is null.");
			return SINCE_BEGINING;
			

		}

		try {

			String fileName = this.getDBName() + ".rpdata";
			Path path = Paths.get(fileName);

			if (Files.exists(path)) {
				sinceString = new String(Files.readAllBytes(path));
				logger.debug("Last sent update sequence " + sinceString
						+ " for " + this.getDBName()
						+ " database obtained from file.");
			}
		} catch (IOException e) {
			logger.error("in getLastSentUpdateSeq() for " + this.getDBName() + ":"
					+ e.getMessage());
		}

		return sinceString;
	}

	/**
	 * Store last sent update seq.
	 *
	 * @param since
	 *            the since
	 * @return true, if successful
	 */
	public boolean storeLastSentUpdateSeq(String since) {
		try {
			String fileName = this.getDBName() + ".rpdata";
			Path path = Paths.get(fileName);
			Files.write(path, since.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
			logger.debug("Storing last sent update sequence (" + since
					+ ") for " + this.getDBName() + " database to file.");

		} catch (NumberFormatException e) {
			logger.error("in storeLastSentUpdateSeq() for " + this.getDBName() + ":"
					+ e.getMessage());
			return false;
		} catch (IOException e) {
			logger.error("in storeLastSentUpdateSeq() for " + this.getDBName() + ":"
					+ e.getMessage());
			return false;
		}

		return true;

	}

}
