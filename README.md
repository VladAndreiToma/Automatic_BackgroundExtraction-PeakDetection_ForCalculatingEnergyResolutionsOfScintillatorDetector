This code aims for now to study the responses of detector under different configuration of DAQ software parameters.
By analysing the emission spectra of Co60 source we get detector responses( scintillator detectors for now ). From there further processing is needed
First we extract the background using the extraction algorhythm that Cern's ROOT - TSpectrum Class provides.
After we get a clean spectra we procceed to extract peaks with the same TSpectrum Class. We introduce some mean value that we might expect from literature and some threshold above which the algorhythm within this method should account for signals in peak calculation
We are interested in the position on the x axis that is correlated with the energy of the gamma ray emitted and present in the spectra.
Then around that "mean" value we perform iterrative fits( optional ) or just a one time fit to get the true mean and sigma for a gaussian function that would describe the peak shape as perfect as possible.
Having these values for both gammas of Co60( cobalt ) we can compute the resolution as (FWHM/MEAN)*GammaRayEnergy.
