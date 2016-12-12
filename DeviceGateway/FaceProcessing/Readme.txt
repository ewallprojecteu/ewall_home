FaceProcessing component
------------------------

This perceptual component processes the video stream from a webcam or a video file. It detects and tracks persons facing the camera and estimates their emotion. It can also store the results in a couchdb server.

Two executables are created:

face_processing: Client for usage with the Device Gateway
face_processing_noDGW: Standalone executable that can open any provided video or webcam

----------------

Examples:
./face_processing http://127.0.0.1:5984/visual
./face_processing_noDGW my-video-file.avi
./face_processing_noDGW 0 (By specifying "0" for the video name it uses the default camera, if not working you may try "1","2","3", etc.)
./face_processing_noDGW my-video-file.avi http://127.0.0.1:5984/visual

Note that the commands with a couchdb address (http://127.0.0.1:5984/visual) require the database "visual" to be already created.
In order to do so, after installing couchdb, go to http://127.0.0.1:5984/_utils/ using your browser and create a database called "visual". You may need to login as administrator to perform this action.

