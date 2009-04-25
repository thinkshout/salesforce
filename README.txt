// $Id$

SALESFORCE MODULE

New architecture for modularity and Druapl 6 compatibility.

This module utilizes SalesForce's Enterprise SOAP client and generic WSDL 
functionality to allow you to work with custom fields without having to 
download your own WSDL file.

It also includes an API architecture which allows for additional modules to 
be easily plugged in (e.g. for webforms, contact form submits, etc).

INSTALLATION:

1) Download, uncompress and situate the module as per usual.

2) Download the salesforce PHP toolkit version 13:

   http://wiki.apexdevnet.com/index.php/PHP_Toolkit

   Place the "soapclient" directory withinin the "toolkit" directory within the
   Drupal module's "salesforce_api directory". You should end up with something
   like: sites/all/modules/salesforce/salesforce_api/toolkit/soapclient

3) Enable the module on admin/build/modules along with at least one of the 
   object modules (salesfoce_node, salesfoce_user). Node is the usual place to 
   start.

QUICKSTART:

1) Visit admin/settings/salesforce and enter your login information in the 
   "Salesforce API Settings" fieldset. Save that configuration.

2) Click on the "Fieldmaps" local task, and create a fieldmap between Drupal and 
   SalesForce objects.

3) If you left the "automatic" box checked, the next time a Drupal object is 
   created, it will create a corresponding object in SalesForce. Alternatively, 
   you can click the "salesforce" tab on any node which you've established a 
   mapping for and manually create a SalesForce object for it.