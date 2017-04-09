# DSVSCA-Core

## Compile

In the root directory, run the following command:

> make

The program and all dependencies should automatically build and compile.

## Running DSVSCA
```
Mandatory arguments:
-v, --video=VIDEO-FILE          Specifies the location of the video file to virtualize.
-s, --sofa=SOFA-FILE            Specifies the location of the SOFA file to use during the virtualization process.

Optional arguments:
-h, --help                      Prints out the help menu specifying all required and optional parameters.
-b, --block-size=BLOCK-SIZE     Specifies the block size used when processing the audio. A smaller block size results in better virtualization but takes longer to process.
-c, --coord-type=TYPE           Specifies the coordinate system used when specifying virtualized speaker placement. The values can be Cartesian or Spherical. The default value used is Cartesian.
-fl, --fl=X,Y,Z                 Specifies the x, y, and z or phi, theta, and radius coordinates of the front left speaker. If this value is not specified, the default value of 1, 1, 0 is used.
-fc, --fc=X,Y,Z                 Specifies the x, y, and z or phi, theta, and radius coordinates of the front center speaker. If this value is not specified, the default value of 1, 0, 0 is used.
-fr, --fr=X,Y,Z                 Specifies the x, y, and z or phi, theta, and radius coordinates of the front right speaker. If this value is not specified, the default value of 1, -1, 0 is used.
-bl, --bl=X,Y,Z                 Specifies the x, y, and z or phi, theta, and radius coordinates of the back left speaker. If this value is not specified, the default value of -1, 1, 0 is used.
-br, --br=X,Y,Z                 Specifies the x, y, and z or phi, theta, and radius coordinates of the back right speaker. If this value is not specified, the default value of -1, -1, 0 is used.
-lfe, --lfe=X,Y,Z               Specifies the x, y, and z or phi, theta, and radius coordinates of the Low-Frequency Efects (subwoofer) speaker. If this value is not specified, the default value of 1, 0, 0 is used.
```

