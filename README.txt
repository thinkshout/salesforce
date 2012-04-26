
SALESFORCE MODULE
-----------------
  Contents of this README:
    ABOUT
    REQUIREMENTS
    INSTALLATION AND CONFIGURATION
    UPDATING / REINSTALLING / ENABLING / DISABLING
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
  objects and Drupal entities. In other words, for each of your supported Drupal
  entities (e.g. node, user, or entities supported by extensions), you can 
  assign Salesforce objects that will be created / updated when the entity is
  saved. For each such assignment, you choose which Drupal and Salesforce fields
  should be mapped to one another.

  This suite also includes an API architecture which allows for additional
  modules to be easily plugged in (e.g. for webforms, contact form submits,
  etc).
  
  For a more detailed description of each component module, see MODULES.txt.


REQUIREMENTS
------------
  1) You need a Salesforce account. Developers can register here:
  
  http://www.developerforce.com/events/regular/registration.php
  
  You will need to know your login data and your security token.

  2) You will need to download your organization's generated Enterprise WSDL file. This must be 
     uploaded to the site prior to entering the connection information.
     (see WORKING WITH WSDL FILES)

  3) PHP to have been compiled with SOAP web services and OpenSSL support, as per:
  
  http://php.net/soap
  http://php.net/openssl

  4) Required modules
     Chaos Tool Suite - http://drupal.org/project/ctools

  5) Recommended modules
     AES encryption - http://drupal.org/project/aes
     (see SOME NOTES ABOUT SECURITY)


INSTALLATION AND CONFIGURATION
------------------------------
  1) Download, uncompress and situate the module as per usual.

  2) Download the Salesforce PHP Toolkit

     The official Force.com PHP toolkit is available from github.
     The recommended method of installing is using git.
     $ git clone git://github.com/developerforce/Force.com-Toolkit-for-PHP.git

     You can also use drush make:
     $ drush make salesforce.make.example
     
     Project homepage:
     https://github.com/developerforce/Force.com-Toolkit-for-PHP

     Place the toolkit in a directory called "toolkit" under your Libraries path for "salesforce".

     The only part of the toolkit needed is the soapclient. The rest may be deleted.

     If you have installed the soapclient successfully, it will be found at a path like:

     sites/all/libraries/salesforce/toolkit/soapclient

  3) If you desire to use the recommend AES Encryption to store your login credentials,
     download and enable that module. Then set it up to use a file for storing the encryption key.

  4) Enable the module on admin/build/modules along with at least Salesforce Entity, so that
     nodes, users, taxonomy terms, and other Drupal entities may be exported.

  5) Assign a WSDL directory and upload your organization's WSDL file
     (admin/config/salesforce/wsdl).

  6) Enter the username, password, and security token with which you want the module to connect
     to Salesforce on the module's main settings page (admin/config/salesforce). 

     It is recommended to use an "API user", distinct from any of the regular users of your 
     Salesforce installation, so that the actions taking by your Drupal integration with Salesforce
     can be distinguished from those of your regular Salesforce users.

  7) While still on admin/config/salesforce, click on the "Fieldmaps" tab, and create a fieldmap 
     between Drupal entities and Salesforce objects. Default fieldmaps for users and nodes have been
     created for an example.

  8) If you check the "Create" tab under Sync with Salesforce, the next time a Drupal entity is
     created, it will create a corresponding object record in Salesforce.
     Alternatively, you can click the Salesforce tab on any entity for which you've
     established a mapping for and manually create a Salesforce object record for it.

UPDATING / REINSTALLING / ENABLING / DISABLING
----------------------------------------------
  If you have previously installed this module on your Drupal site and are 
  upgrading, you need to do the following to update the module.

  0) ALWAYS backup your site's code and your database.

  1) Download the latest version of Salesforce Suite and then run update.php,
     or use the following drush command: "drush up salesforce"

  ### Upgrading from 6.x or older
  
  Currently there is no upgrade path from the 6.x-2.x to 7.x-2.x branch of the module.
  You would have to recreate all fieldmaps and linkages manually.
  If you would like to help create an upgrade path, see http://drupal.org/node/1199022.

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
  fail-safe so that data is not lost. Failed API transactions will be queued for 
  future reprocessing. In addition, sf_queue is highly configurable to allow 
  administrators maximum flexibility in setting up the queue.


SOME NOTES ABOUT SECURITY
--------------------------
  By default all Salesforce credentials are stored in the variables table,
  unencrypted. If this is a problem for you, this module supports encryption via
  aes module http://drupal.org/project/aes. You will need to create a directory
  outside your webroot (you can use the same one you used for your WSDL) wherein
  your encryption key will be stored. Your credentials will thus forth be
  encrypted as securely as AES allows. PLEASE NOTE: your data is still only as
  secure as your network. It may be possible for a savvy attacker to access your
  data at any of various points between your Drupal site and Salesforce.com. As
  this always, you should educate yourself about the risks involved before storing
  and transferring sensitive data across the Internet.


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
  In addition to out of the box support for various FieldAPI fields (in sf_entity), 
  an extensible framework is available for developers to build or alter support for contrib 
  modules. See sf_contrib for examples and best practices, as well as hooks.php 
  for documentation of the available integration points.


TROUBLESHOOTING
---------------
  Troubleshooting connection errors:
    * Are you using the right WSDL? Generate a new one.
    * Disable SOAP WSDL cache.
    * If you're switching from a sandbox to a live Salesforce instance, make sure that you switched
      accounts when generating the WSDL.
    * Can you connect to the sandbox?
    * Are you logging everything? Have you checked watchdog?
    * Re-enter the username, password, and security token.
    * Try another Salesforce user's credentials.

  In Salesforce:
    * Regenerate the WSDL.
    * If you're switching from a sandbox to a live Salesforce instance, make sure you're in the
      right account.
    * Make sure the IP of the Drupal site's host is not blocked.
    * Go to: [Your Name] > Setup > Users. Check login attempts of the user you are using to connect.
      See if there are any API attempts listed.
    * Reset the credentials associated with the user.

  In the Salesforce API module:
    * Replace WSDL file with the one you want to use
      (admin/settings/salesforce/wsdl)

  Install Devel:
    * execute PHP (devel/php):
        $somevariable = salesforce_api_connect();
        dpm($somevariable);
      This will show whether the SOAP toolkit was able to connect to your instance, using the
      credentials currently in the variables table.

  In the file salesforce.module:
    * In salesforce_api_login(), find the line
        $sf->client->createConnection($wsdl)
    * Before that line put dpm($wsdl);
    * Make sure that your WSDL file is named "enterprise.wsdl.xml".

  PHPinfo():
    * Check that SOAP is enabled
    * Check that soap.wsdl_cache_enabled is FALSE
    * Check that the openssl extension is enabled. It must be compiled into your PHP build.

  If you have shell access:
    * curl https://login.salesforce.com to make sure your machine can connect.


REPORTING BUGS
--------------
  Bug reports should adhere to Drupal standards.

  Note that the Drupal 7.x-2.x version is still unstable and not yet ready for production use.
  There are known issues with it, and both bug reports and patches are welcome.

  Before creating a new issue, please:
    * Review existing issues for 7.x-2.x at http://drupal.org/project/issues/salesforce
      and see if your issue already has been reported. If so, comment on the existing issue.
    * Include the PHP error message, if there is any.

  Additionally, once your issue is reported, the maintainers may ask you to do the following to
  help debug the issue:
    * Use latest code from Git on a *new* Drupal install.
    * Turn on "log all Salesforce activity" and include any relevant watchdog
      errors.
    * Install a generated WSDL file and Clear your WSDL cache
      (see WORKING WITH WSDL FILES)
    * Confirm PHP's SOAP support.
    * Confirm whether you were able to successfully:
      - Connect to Salesforce.
      - Save credentials.
      - Login to https://login.salesforce.com/ with those credentials.
      - Load the test/demo page.
      - Create a fieldmap.
    * Go through the Troubleshooting steps above and include any relevant info.
    * Include all the information from the list above, and system information,
      including operating system, PHP version, Apache version.
