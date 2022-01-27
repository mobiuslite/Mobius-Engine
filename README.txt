Graphics Project 3 by Ethan Robertson

Video link: https://youtu.be/mvn2OGxhdPk

Model sources:
	Models created in Minecraft, exported with Mineways

HOW TO RUN:
	-Open the visual studio solution file and press run in either debug or release on x64
	-Note: By default it is run in debug mode, if you have framerate issues please run in release mode
	
CONTROLS:
	*W-A-S-D / E-Q to move camera
	*LEFT CLICK and move mouse to look around
	*N to switch between day and night
	*F1 to enable debugging mode (Shows debug sphere)
	*F3 to disable/enable lighting (Only in debug mode)
	*F4 to show normals (Only in debug mode)

LOCATION OF ENHANCED CODE:

	ILoveOpenGl/Physics
	Emission creation: theMain.cpp lines, 1050-1074

	Tangent/BiTangent calculations
	cVAOManager.cpp lines 424-477
	