
SALESFORCE MODULE
-----------------
  Contents of this README:
    ABOUT
    REQIREMENTS
    INSTALLATION
    UPDATING / REINSTALLING / ENABLING / DISABLING
    QUICKSTART
    PREMATCHING
    EXPORT QUEUE
    WORKING WITH WSDL FILES
    NOTIFICATIONS
    EXTENDING
    TROUBLESHOOTING
    REPORTING BUGS


ABOUT
-----
  This module suite implements a mapping functionality between Salesforce
  Objects and Drupal entities. In other words, for each of your supported Drupal
  entities (e.g. node, user, or entities supported by extensions), you can 
  assign Salesforce objects that will be created / updated when the entity is
  saved. For each such assignment, you choose which Drupal and Salesforce fields
  should be mapped to one another.

  This suite also includes an API architecture which allows for additional
  modules to be easily plugged in (e.g. for webforms, contact form submits,
  etc).
  
  For a more detailed description of each component module, see MODULES.txt


REQUIREMENTS
------------
  1) You need a salesforce account. Developers can register here:
  
  http://www.developerforce.com/events/regular/registration.php
  
  You will need to know your login data and your security token.

  2) PHP needs the SOAP web services kit installed, as per:
  
  http://php.net/soap

  3) Required modules
     Chaos Tool Suite - http://drupal.org/project/ctools

  4) Recommended modules
     Libraries API - http://drupal.org/project/libraries


RECOMMENDED
-----------

  1) Download and install your organization's generated Enterprise WSDL file.
     (see WORKING WITH WSDL FILES)

  2) AES encryption
     http://drupal.org/project/aes
     (see SOME NOTES ABOUT SECURITY)


INSTALLATION
------------
  1) Download, uncompress and situate the module as per usual.

  2) Download the Salesforce PHP Toolkit

     The official Force.com PHP toolkit is available from github.
     The recommended method of installing is using git.
     $ git clone git://github.com/developerforce/Force.com-Toolkit-for-PHP.git

     You can also use drush make:
     $ drush make salesforce.make.example
     
     Project homepage:
     https://github.com/developerforce/Force.com-Toolkit-for-PHP

     If "libraries" module is installed, place the "soapclient" directory in
     "salesforce/soapclient" in your libraries path.
     You should end up with:

     sites/all/libraries/salesforce/soapclient

     Otherwise, if "libraries" modules is not installed, place the "soapclient"
     directory within the "toolkit" directory in "salesforce_api".
     You should end up with:

     sites/all/modules/salesforce/salesforce_api/toolkit/soapclient

  3) Enable the module on admin/build/modules along with at least one of the
     object modules (sf_node, sf_user, sf_contrib). sf_node is the usual place
     to start.

  4) Assign a WSDL directory and upload your organization's WSDL file
     (admin/settings/salesforce and admin/settings/salesforce/wsdl).


UPDATING / REINSTALLING / ENABLING / DISABLING
----------------------------------------------
  If you have previously installed this module on your Drupal site and are 
  upgrading, you need to do the following to update the module.

  0) ALWAYS backup your site's code and your database.

  1) Download the latest version of Salesforce Suite and then run update.php,
     or use the following drush command: "drush up salesforce"

  ### Older versions

  If you are using an older version of the Salesforce 2.x module (2009 or 
  earlier), follow these directions:
  
  0) Backup your site's code and database. Download your WSDL, which would be
     in sites/all/modules/salesforce_api/wsdl/enterprise.wsdl

  1) Download the latest version of Salesforce Suite and then run update.php,
     or use the following drush command: "drush up salesforce"
  
  2) You will have to update your WSDL directory. It is recommended that your 
     WSDL file be stored outside your webroot. After enabling the new version 
     of Salesforce, visit admin/settings/salesforce and set the WSDL directory 
     and click "Save configuration". You will then be prompted to upload a new 
     WSDL file (admin/settings/salesforce/wsdl).
     
  Note that your fieldmaps may need adjustment after upgrading.

QUICKSTART
----------
  0) Minimal: enable salesforce_api and sf_node or sf_user.
     Kitchen sink: enable salesforce_api, sf_node, sf_user, sf_prematch, and 
     sf_queue
  
  1) Visit admin/settings/salesforce and enter your login information in the
     "Salesforce API Settings" fieldset and "WSDL directory".
     Save that configuration.

  2) Upload your organization's generated WSDL file.

  3) Click on the "Fieldmaps" local task, and create a fieldmap between Drupal
     and Salesforce objects.

  3) If you left the "automatic" box checked, the next time a Drupal object is
     created, it will create a corresponding object in Salesforce.
     Alternatively, you can click the "salesforce" tab on any node which you've
     established a mapping for and manually create a Salesforce object for it.


PREMATCHING
-----------
  The module sf_prematch provides administrators the ability to set up duplicate
  prevention criteria on each fieldmap. When sf_permatch is enabled, 
  administrators will be directed to set up matching criteria after creating a 
  fieldmap. Each time a Salesforce export is triggered, this criteria will be 
  used to identify any pre-existing records. If any matching record is found, 
  the matched record will be updated instead of a new record being created. The 
  impetus for this is to reduce the database management workload for 
  Salesforce.com administrators.


EXPORT QUEUE
------------
  The module sf_queue implements a queueing system for exports. Since Salesforce 
  API's create and update functions can modify up to 200 records at a time, this 
  module offers significant efficiencies for users trying to minimize their API 
  usage. Further, if an account's API limit is exceeded, the queue provides a 
  failsafe so that data is not lost. Failed API transactions will be queued for 
  future reprocessing. In addition, sf_queue is highly configurable to allow 
  administrators maximum flexibility in setting up the queue.


SOME NOTES ABOUT SECURITY
--------------------------
  By default all SalesForce credentials are stored in the variables table,
  unencrypted. If this is a problem for you, this module supports encryption via
  aes module http://drupal.org/project/aes. You will need to create a directory
  outside your webroot (you can use the same one you used for your WSDL) wherein
  your encryption key will be stored. Your credentials will thus forth be
  encrypted as securely as AES allows. PLEASE NOTE: your data is still only as
  secure as your network. It may be possible for a savvy attacker to access your
  data at any of various points between your Drupal site and SalesForce.com. As
  this always, you should educate yourself about the risks involved before storing
  and transferring sensitive data across the internet.


WORKING WITH WSDL FILES
-----------------------
  If you do not upload a WSDL file, Salesforce module will use a default .wsdl
  file (soapclient/enterprise.wsdl.xml), which may not be compatible with your
  organization's Salesforce installation or the current Salesforce API. It is
  highly recommended that you supply your own enterprise wsdl file via WSDL
  administration at admin/settings/salesforce/wsdl.

  Every time your Salesforce schema changes, you will need to regenreate your
  WSDL and upload it to your Drupal site if the changes affect mapped Salesforce
  objects. Changing Salesforce schema may result in breaking your fieldmaps, so
  please do so with extreme caution.

  When switching between wsdl files, keep in mind that PHP's SoapClient is
  caching wsdl information. Though PHP's SOAP WSDL cache should be cleaned when
  you upload a new WSDL file, you can permenantly disable caching of wsdl
  information by adding this line to your settings.php:

  ini_set('soap.wsdl_cache_enabled',  '0');

  You can control the life time of your cache by adding this line to your
  settings.php:

  ini_set('soap.wsdl_cache_ttl',      '0');

  For more information on SoapClient refer to
  http://php.net/manual/en/book.soap.php


NOTIFICATIONS
-------------
  Salesforce Outbound Messages (referred to as Notifications) are XML messages
  from Salesforce that can be sent based on Salesforce Workflow actions to any
  web endpoint. The included module sf_notifications handles processing of any
  such Notifications.

  To allow Drupal to respond to Notifications, enable the sf_notifications
  module as you would any other module, and point your Outbound Message(s) to
  the notification endpoint:

    http://example.com/sf_notifications/endpoint
    
  Configuring Salesforce Outbound Messages and Workflow is outside the scope of
  this documentation.
  
  SF_notifications will expose your existing fieldmaps to function as
  handlers for notifications. You can configure which of your fieldmaps should
  be active, and set conditions upon which the Notifications will be used to
  create Drupal objects. One application of Notifications is to implement full
  two-way synchronization between Salesforce and Drupal.  


EXTENDING
----------
  Support for mapping fields from commonly used modules like location and cck is 
  provided by sf_contrib module. If you use node_location, user_location, 
  cck_location, or any cck other fields and you'd like them to be available for 
  export, you need to enable this module.
  
  In addition to out of the box support for various cck fields, an extensible 
  framework is available for developers to build or alter support for contrib 
  modules. See sf_contrib for examples and best practices, as well as hooks.php 
  for documentation of the available integration points.


TROUBLESHOOTING
---------------
  Troubleshooting connection errors:
    * Are you using the right WSDL? Generate a new one.
    * Disable SOAP WSDL cache.
    * If you’re switching from sandbox to enterprise make sure that you switched
      accounts when generating the wsdl
    * Can you connect to the sandbox?
    * Are you logging everything? Have you checked watchdog?
    * Re-enter the token, user name and pass
    * Try another user

  In Salesforce:
    * Regnerate the WSDL
    * If you’re switching from sandbox to enterprise, make sure you're in the
      right account
    * Make sure the ip of server is not blocked
    * Go to: setup > users. Check login attempts of the user you are trying to
      connect with
    * Reset the credentials associated with the user

  In the Salesforce API module:
    * Replace WSDL file with the one you want to use
      (admin/settings/salesforce/wsdl)

  Install Devel:
    * excecute php:
      $somevariable = salesforce_api_connect();
      dpm($somevariable);

  In the file salesforce.module:
    * In salesforce_api_login(), find the line
      $sf->client->createConnection($wsdl)
    * Before that line put dpm($wsdl);
    * Make sure that your WSDL file is named "enterprise.wsdl.xml"

  PHPinfo():
    * Check that SOAP is enabled
    * Check that soap.wsdl_cache_enabled is FALSE
    * Check that openssl is enabled.

  If you have shell access:
    * curl https://login.salesforce.com to make sure your machine can connect.


REPORTING BUGS
--------------
  Bug reports should adhere to Drupal standards.

  Before creating a new issue, please:
    * Review existing issues at http://drupal.org/project/issues/salesforce
    * Use latest code from CVS on a *new* Drupal install
    * Include the PHP error message
    * Turn on "log all salesforce activity" and include any relevant watchdog
      errors
    * Install a generated WSDL file and Clear your WSDL cache
      (see WORKING WITH WSDL FILES)
    * Confirm php's SOAP support
    * Confirm whether you were able to successfully:
      - Connect to Salesforce
      - Save credentials
      - Login to https://login.salesforce.com/ with those credentials
      - Load the test/demo page
      - Create a fieldmap
    * Go through the Troubleshooting steps above and include any relevant info
    * Include all the information from the list above, and system information,
      including operating system, php version, apache version
