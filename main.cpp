// instantiate an ObjInput object with cmdline filename
// that's it

#include "ObjInput.h"

#include <string>

int main(int argc, char* argv[])
{
    // read in initialization file, create Objects
    std::string infname;
    std::string outfname;

    // get init file from args (default "init.dat")
    if (argc >= 2)
    {
        infname = argv[1];
	}

    if ((argc == 1) || !infname.compare("-?") || !infname.compare("/?") || !infname.compare("-help") || !infname.compare("/help"))
    {
        printf("Usage:  obgen inputfilename [outputfilename]\n");
        printf("\tFile format:\n");
        printf("\t\tnumObjects = [1e6] minMass = [1], maxMass = [1e6], massDist = [\"uniform\"], ");
        //printf("momMin = [1], momMax = [1e3], momDist = [\"uniform\"], ");
        printf("volShape = [\"spherical\"], vol1 = [1e6], vol2 = [1e6], vol3 = [1e6], volDist = [\"exponential\"], lambda = [5000] \n\n");
        printf("\t\tnumObjects (int), others double or strings (below), lambda used in exponential distributions\n");
        printf("\t\tmassDist values: \"uniform\", \"uniform low\", \"uniform lower\", \"exponential\"\n");
        printf("\t\tmomDist values: \"uniform\", \"exponential\"\n");
        printf("\t\tvolShape values: \"spherical\" (vol1 is radius, others ignored), \n");
        printf("\t\t\t\"rectilinear\" (vol1/2/3 are x, y, z mag from center), \n");
        printf("\t\t\t\"cylindrical\" (vol1 is radius, vol2 is z, vol3 is ignored)\n");
        printf("\t\tvolDist values: \"uniform\", \"uniform low\", \"uniform lower\", \"exponential\"\n");
        return 1;
    }
	
	if (argc >= 3)
    {
        outfname = argv[2];
	}
	else
	{
		outfname = infname;
		size_t n = outfname.rfind('.');

		if (n != std::string::npos)
		{
			size_t diff = outfname.length() - n;
			outfname.replace (n, diff, ".ascii");

			if (outfname.compare(infname) == 0)
			{
				// yell at user and tell them to use  different input extension
				printf("Input extension:  Do not use \".ascii\" since that's the default output extension\n");
				return 1;
			}
		}
		else
		{
			outfname += std::string(".ascii");
		}
	}

    ObjInput oinp(infname, outfname);
	return oinp.CreateObjectsAndWrite();
}
