import os
import re
import shutil

#get ranked-choice list of country codes
def getCountryHierarchy():
  countryHierarchy = []
  cont = True
  while(cont):
    newCountryCode = input("Enter a country code, e.g. USA, or enter nothing to continue:\n")
    if(newCountryCode):
      countryHierarchy.append(newCountryCode)
    else:
      cont = False
  if(countryHierarchy == []):
    return ['USA', 'Japan', 'Europe']
  return countryHierarchy

#get information for folder
def getFolderInfo():
  cont = True
  while(cont):
    directory = input("Enter the absolute path of your ROM directory:\n")
    if(os.path.exists(directory) and os.path.isdir(directory)):
      cont = False
    else:
      print("That is not a valid path!")
  newFolderName = input("\nEnter the name of your new folder (do NOT include path information):\n")
  if not newFolderName:
    newFolderName = "cleaned"
  else: #this is entirely for aesthetics
    print()
  destination = directory + "/" + newFolderName
  return directory, destination

#remove region and language info from a filename
def removeRegionAndLang(string):
  string = re.sub(r' \([^)]*\)', '', string, count = 1)
  string = re.sub(r' \(En,[^)]*\)', '', string)
  return string

#generate a dictionary of roms, grouped by filename w/o language or region
def generateCache(sourceDirectory):
  cache = {}
  for filename in os.listdir(sourceDirectory):
    if(os.path.isfile(f"{sourceDirectory}/{filename}")):
      noRegion = removeRegionAndLang(filename)
      if not noRegion in cache:
        cache[noRegion] = []
      cache[noRegion].append(filename)
  return cache

#iterate over a list of filenames, selecting one based on most-preferred region
def selectBestCountry(cacheEntry, countryHierarchy, keepInvalid = False):
  for country in countryHierarchy:
    for entry in cacheEntry:
      if (country in entry):
        return entry
  if(keepInvalid):
    return entry
  else:
    return None

#get the list of chosen filenames
#also check if the user wants to include files only found in regions they didn't select
def getList(cache, countryHierarchy):
  keepInvalidInput = input("Do you want to keep games that are only available in regions you did not select? (y/n)\n")
  if(keepInvalidInput in ['Y', 'y', 'Yes', 'yes']):
    keepInvalid = True
  else:
    keepInvalid = False
  
  lst = []
  for k,v in cache.items():
    best = selectBestCountry(v, countryHierarchy, keepInvalid)
    if(best):
      lst.append(best)
  return lst

#move the selected files into the destination directory
def moveRoms(listOfRoms, source, destination):
  print("Moving ROMs...")
  if not os.path.exists(destination):
    os.mkdir(destination)
  for i in listOfRoms:
    newpath = shutil.move(f"{source}/{i}", destination)
  print("Done!")

#actually run the code
def main():
  countryHierarchy = getCountryHierarchy()
  source, destination = getFolderInfo()
  cache = generateCache(source)
  listOfChosenRoms = getList(cache, countryHierarchy)
  moveRoms(listOfChosenRoms, source, destination)
  input("PRESS ENTER TO EXIT...")

if __name__ == "__main__":
    # execute only if run as a script
    main()