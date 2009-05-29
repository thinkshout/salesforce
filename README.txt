// $Id$

SALESFORCE MODULE

New architecture for modularity and Druapl 6 compatibility.

This module utilizes Salesforce's Enterprise SOAP client and generic WSDL 
functionality to allow you to work with custom fields without having to 
download your own WSDL file.

It also includes an API architecture which allows for additional modules to 
be easily plugged in (e.g. for webforms, contact form submits, etc).

REQUIREMENTS:

1) You need a salesforce account. Developers can register here:

http://www.developerforce.com/events/regular/registration.php

You will need to know your login data and your security token.

2) PHP needs the SOAP web services kit installed, as per:

http://php.net/soap


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
   Salesforce objects.

3) If you left the "automatic" box checked, the next time a Drupal object is 
   created, it will create a corresponding object in Salesforce. Alternatively, 
   you can click the "salesforce" tab on any node which you've established a 
   mapping for and manually create a Salesforce object for it.

WORKING WITH WSDL FILES

Salesforce module will use a default .wsdl file
(salesforce_api/toolkit/soapclient/enterprise.wsdl.xml). Alternatively, you can
supply your own enterprise wsdl file by placing it in the salesforce_api/wsdl/ 
directory (salesforce_api/wsdl/enterprise.wsdl - no .xml at the end of the file
name).

When switching between wsdl files, keep in mind that PHP's SoapClient is caching
wsdl information. You can turn off caching of wsdl information by adding this 
line to your settings.php:

ini_set('soap.wsdl_cache_enabled',  '0');

You can control the life time of your cache by adding this line to your 
settings.php:

ini_set('soap.wsdl_cache_ttl',      '0');

For more information on SoapClient refer to http://php.net/manual/en/book.soap.php
