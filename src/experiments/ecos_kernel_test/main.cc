#include <iostream>
#include <cstdlib>

#include "cpn/CampaignManager.hpp"
#include "campaign.hpp"

int main(int argc, char **argv)
{
	EcosKernelTestCampaign c;
	if (fail::campaignmanager.runCampaign(&c)) {
		return 0;
	} else {
		return 1;
	}
}
