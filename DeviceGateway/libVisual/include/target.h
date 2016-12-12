#ifndef TARGET_H
#define TARGET_H

#include <opencv2/core/core.hpp>

//TODO: convert particles to vector and move this to PF tracker
#define NP						50 //Number of particles for PF tracker

//TODO: remove State and use cv::RotatedRect
typedef struct State
{
	double	x;
	double	y;
	double	size;
	double	angle;
}State;

typedef struct particle
{
	State	state;
	double	w;
}particle;

typedef struct
{
	// Fields used in JSON reporting
	long		target_id;		// Auto-incremented target number

	// Size and position
	cv::RotatedRect face;		// TODO: Replace the State with this
	double		spread;			// Particle spread (Indicating position confidence, currently set to 0 for full confidence)

	// Emotion recognition
	std::string emotion;		// Detected emotion
	double      emotionConf;    // Confidence of detection

	// Gender and age
	int         ageEst;         // Estimated age of target in years
	double      ageEstConf;     // Confidence of target age (0 to 1)
	double		gender;			// Average gender decision (0 = male, 1 = female, 0.5 = unspecified)


	// Other internal fields for tracker

	//TODO: replace particle positions and weights with vectors
	particle	particles[NP];	// Particles

	cv::Mat		colorModel;		// Colour model
	State		xo;				// Estimated state
	cv::Mat		C;				// Object model covariance matrix
	double		Lmax;			// Maximum likelihood between particles
	double      LcompMax[3];	// Maximum likelihood of the individual components, 0 face, 1 color, 2 motion
	int			reinitDuration;	// Age in frames since latest re-initialisation
	double		Linit;			// Initial maximul likelihood of particles

	bool        isFrontal;      // Whether the target is currently facing the camera
	bool        isGlancing;     // Whether the target was frontal during the last second
	std::vector<int64> glance_times; // Pairs of glances starting/ending
	int64       facing_duration;// Total duration that target was facing the camera
	int64		facing_time;	// Time in ms when target was last facing the camera
	int         facing_dist;	// Estimated distance from camera, 0=far,1=medium,2=near

	cv::Rect	trackWindow;	// The current bounding rectangle of the target
	int64		created_time;	// Time in ms when this target was created
	int64		detected_time;	// Time in ms of most recent face detection of the target
	cv::Rect	latestDetection;// Rectangle of the most recent face detection of the target
	cv::Mat		latestFace;		// Stores the most recent preprocessed and normalised face
	std::vector<int> genderCounts;	// Stores the number of individual gender decisions
	std::vector<double> genderConf;	// Stores the sum of confidences of the above decisions
	int         agesCount;      // Stores the number of age decisions
	double      agesConfSum;	// Stores the sum of confidences for the age decisions
	double      agesWeightedSum;// Stores the sum of confidences*ages
	int64		recognised_time;// Time in ms of last recognised face
	cv::Point	eyes[2];		// Position of left/right eyes

} targetStruct;


#endif /*TARGET_H*/