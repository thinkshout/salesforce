IT IS HIGHLY RECOMMENDED THAT YOU INSTALL LIBRARIES API
http://drupal.org/project/libraries

If Libraries API is not installed, this directory is where the PHP soapClient
directory goes.

Download the Salesforce PHP Toolkit:

   https://github.com/messageagency/salesforce
   or 
   git://github.com/messageagency/salesforce.git

   If "libraries" module is installed, place "soapclient" in
   "salesforce/soapclient" in your libraries path.
   You should end up with something like:
   sites/all/libraries/salesforce/soapclient

   If "libraries" modules is not installed, place the "soapclient" directory
   within the "toolkit" directory in "salesforce_api". 
   You should end up with something like:
   sites/all/modules/salesforce/salesforce_api/toolkit/soapclient
