<?php
// $Id$

/**
 * @file
 * integration of Drupal and Salesforce.com.
 * Original author: Steve Mckenzie
 * Current maintainer: Victor Kane
 */

/**
 *  salesforce include + connecting via SOAP
 */
function salesforce($login_return = false) {
  static $included;

  $admin = user_access('administer salesforce');

  $username = variable_get('salesforce_user', NULL);
  $password = variable_get('salesforce_password', NULL);

  if ($username && $password) {
    if (!$included) {
      require_once('salesforce.php');
      $included = true;
    }

    // create client and login
    $salesforce = new salesforce($_SERVER['DOCUMENT_ROOT'] . base_path() . drupal_get_path('module', 'salesforce') . '/includes/partner.wsdl');
    $login = $salesforce->login($username, $password);

    if (!$login) {
      if ($admin) {
        drupal_set_message(t('Failed to authenticate on SalesForce.com'), 'error');
        //form_set_error('salesforce_name', t('is the username correct?'));
        //form_set_error('salesforce_password', t('is the password correct?'));
        //drupal_goto('admin/settings/salesforce');
      }

      return false;
    }

    //set batch size header
    $batch_size = new soapval('batchSize', null, 2);
    $salesforce->setHeader('QueryOptions', array($batch_size));

    if ($login) {
      if ($login_return) {
        return $login;
      } else {
        return $salesforce;
      }
    }
  } else {
    if ($admin) {
      drupal_set_message(t('The SalesForce module is currently not configured properly. A username / password to connect to SalesForce is required and not currently supplied. A site administrator should be contacted in regards to this error.'), 'error');
      if (user_access('administer salesforce')) {
        drupal_goto('admin/settings/salesforce');
      }
    }
  }
}

/**
 *  handle contacts in salesforce
 */
function salesforce_contact($op, $account, $params = array()) {
  $salesforce = salesforce();
  $admin = user_access('administer salesforce');

  switch ($op) {
    case 'insert': case 'update':
      $contact = new sObject('Contact', null,
      array(
      'Company' => $account->company,
      'FirstName' => $account->first_name,
      'LastName' => $account->last_name,
      'Phone' => $account->phone,
      'Fax' => $account->fax,
      'Email' => $account->mail,
      ));

      // add any additional params to the contact object
      foreach ($params as $param => $param_value) {
        $contact->$param = $param_value;
      }

      // query salesforce to check if the user already has a contact with the supplied email
      $contact_lookup = $salesforce->query("SELECT id FROM contact WHERE email = '$account->mail'");

      // if we get back multiple results, we only bother looking at the first one.
      if (is_array($contact_lookup['records'])) {
        foreach ($contact_lookup['records'] as $row) {
          $contact_id = $row->id;
          break;
        }
      } else {
        $contact_id = $contact_lookup['records']->id;
      }

      // insert method
      if ($op == 'insert'  && count($contact_lookup['records']) == 0) {
        $result = $salesforce->create($contact);

        if ($result['success'] == 'true') {
          _salesforce_insert('contact_id', $result['id'], $account);
          if (user_access('administer salesforce')) {
            drupal_set_message(t('a contact was created in salesforce for the user <strong>@user</strong>', array('@user' => "$account->first_name $account->last_name")));
            return array('id' => $result['id'], 'status' => 'inserted');
          }
        } else {
          $error = 'contact_insert';
        }
      }
      // update method
      else {
        if (!$account->salesforce['contact_id']) {
          $account->salesforce['contact_id'] = $contact_id;;
          _salesforce_insert('contact_id', $account->salesforce['contact_id'], $account);
        }
        $contact->id = $account->salesforce['contact_id'];
        // why am i unsetting this you may ask? i'm not sure but salesforce tells me too... wtf?
        unset($contact->values['Company']);

        $result = $salesforce->update($contact);

        if ($result['success'] == 'true') {
          if ($admin) {
            drupal_set_message(t('updated contact in salesforce for the user <strong>@user</strong>', array('@user' => "$account->first_name $account->last_name")));
            return array('status' => 'updated');
          }
        } else {
          switch ($result['errors']['statusCode']) {
            default:
              $error = 'contact_update';
              break;
          }
        }
      }

      // handle error control and log errors
      if ($error) {
        if ($admin) {
          drupal_set_message(t('an error occured <strong>[%error]</strong> during a salesforce contact task for the user %user with the messasge - "%message"', array('%user' => theme('username', $account), '%message' => $result['errors']['message'], '%error' => $error)), 'error');
        }
        _salesforce_log_user($error, $result['errors']['message'], $account);
        return array('error' => $error);
      }
      break;
  }
}

/**
 *  handle leads in salesforce
 */
function salesforce_lead($op, $account, $params = array()) {
  // grab a salesforce api connection
  $salesforce = salesforce();

  // avoid fatal error on salesforce query with null element in query
  // catch sf authentication problem (drupal message already set)
  if (!$salesforce) {
    return;
  }

  $admin = user_access('administer salesforce');

  switch ($op) {
    case 'insert': case 'update': default:
      // create the main salesforce object
      $lead = new sObject('Lead', null,
      array(
      'Company' => $account->company,
      'FirstName' => $account->first_name,
      'LastName' => $account->last_name,
      'Phone' => $account->phone,
      'Fax' => $account->fax,
      'Email' => $account->mail,
      ));

      // add any additional params to the lead object
      foreach ($params as $param => $param_value) {
        $lead->values[$param] = $param_value;
      }

      // query salesforce to check if the user already has an account with the supplied email
      $lead_lookup = $salesforce->query("SELECT id FROM lead WHERE email = '$account->mail'");

      // if we get back multiple results, we only bother looking at the first one.
      if (is_array($lead_lookup['records'])) {
        foreach ($lead_lookup['records'] as $row) {
          $lead_id = $row->id;
          break;
        }
      } else {
        $lead_id = $lead_lookup['records']->id;
      }

      // insert method
      if ($op == 'insert' && count($lead_lookup['records']) == 0) {
        $result = $salesforce->create($lead);

        if ($result['success'] == 'true') {
          _salesforce_insert('lead_id', $result['id'], $account);
          if (user_access('administer salesforce')) {
            drupal_set_message(t('a lead was created in salesforce for the user <strong>@user</strong>', array('@user' => "$account->first_name $account->last_name")));
            return array('id' => $result['id'], 'status' => 'inserted');
          }
        } else {
          $error = 'lead_insert';
        }
      }
      // update method
      else {
        if (!$account->salesforce['lead_id']) {
          $account->salesforce['lead_id'] = $lead_id;
          _salesforce_insert('lead_id', $account->salesforce['lead_id'], $account);
        }
        $lead->id = $account->salesforce['lead_id'];

        $result = $salesforce->update($lead);

        if ($result['success'] == 'true') {
          if ($admin) {
            drupal_set_message(t('updated lead in salesforce for the user <strong>@user</strong>', array('@user' => "$account->first_name $account->last_name")));
            return array('status' => 'updated');
          }
        } else {
          switch ($result['errors']['statusCode']) {
            case 'CANNOT_UPDATE_CONVERTED_LEAD':
              // do an update contact object but first find it..
              return salesforce_contact('update', $account, $params);

            case 'INVALID_ID_FIELD':
              if ($admin) {
                db_query("DELETE FROM {salesforce_users} WHERE lead_id = '%s'", $account->salesforce['lead_id']);
                drupal_set_message(t('the lead for the user <strong>@user</strong> was removed from salesforce', array('@user' => "$account->first_name $account->last_name")));
                return array('status' => 'lead_removed_from_salesforce');
              }
              break;

            default:
              $error = 'lead_update';
              break;
          }
        }
      }

      // handle error control and log errors
      if ($error) {
        if ($admin) {
          drupal_set_message(t('an error occured <strong>[%error]</strong> during a salesforce lead task for the user %user with the messasge - "%message"', array('%user' => theme('username', $account), '%message' => $result['errors']['message'], '%error' => $error)), 'error');
        }
        _salesforce_log_user($error, $result['errors']['message'], $account);
        return array('error' => $error);
      }
      break;
  }
}

/**
 *  handle events (history) in salesforce
 */
function salesforce_event($op, $subject, $message, $account = NULL) {
  $account = _salesforce_select_account($account);
  $salesforce = salesforce();
  if (!$salesforce) return array('error' => 'NO_SALESFORCE_CONNECTION');

  $admin = user_access('administer salesforce');

  // contacts before leads
  if ($account->salesforce['contact_id']) {
    $id = $account->salesforce['contact_id'];
  } else if ($account->salesforce['lead_id']) {
    $id = $account->salesforce['lead_id'];
  } else {
    return array('error' => 'NO_CONTACT_OR_LEAD_ID');
  }

  // currently only setting up insert
  switch ($op) {
    case 'insert':
      $activityDate = new soapval('ActivityDateTime', 'dateTime', (string) $salesforce->getServerTimestamp());
      $event = new sObject('Event', NULL, array(
      'WhoId' => $id,
      $activityDate,
      'Subject' => t('%subject - %date', array('%subject' => $subject, '%date' => format_date(time(), 'large'))),
      'Description' => $message,
      'DurationInMinutes' => 1,
      ));
      break;
  }

  $result = $salesforce->create($event);

  if ($result['success'] == 'true') {
    if ($admin) {
      drupal_set_message(t('an event was created in salesforce for the user <strong>@user</strong>', array('@user' => "$account->first_name $account->last_name")));
      return array('id' => $result['id'], 'status' => 'inserted');
    }
  } else {
    switch ($result['errors']['statusCode']) {
      /*
      case 'CANNOT_UPDATE_CONVERTED_LEAD':
      $contact_lookup = $salesforce->query("select id from contact where email = '$account->mail'");
      if (count($contact_lookup['records']) > 0) {
      $account->salesforce['contact_id'] = $contact_lookup['records']->id;
      } else {
      $error = 'event_insert';
      }
      break;
      */
      default:
        $error = 'event_insert';
        break;
    }
  }

  // handle error control and log errors
  if ($error) {
    if ($admin) {
      drupal_set_message(t('an error occured <strong>[%error]</strong> during a salesforce event task for the user %user with the messasge - "%message"', array('%user' => theme('username', $account), '%message' => $result['errors']['message'], '%error' => $error)), 'error');
    }
    _salesforce_log_user($error, $result['errors']['message'], $account, 1, serialize($event));
    return array('error' => $error);
  }
}

/**
 *  select from the salesforce contact table
 */
function salesforce_account_select($account = NULL, $params = array(), $cache = true) {
  $salesforce = salesforce();
  $account = _salesforce_select_account($account);

  // make sure we're dealing with a contact
  if (!$account->salesforce['contact_id']) return array('error' => 'NO_CONTACT_ID');

  $defaults = array('accountId');
  $params = array_merge($defaults, $params);
  // query salesforce for the contact's account_id
  $contact = $salesforce->query("SELECT ". implode(',', $params) ." FROM contact WHERE id = '". $account->salesforce['contact_id'] ."'");
  $id = $contact['records']->values['AccountId'];
  if ($id) {
    if ($cache) {
      _salesforce_insert('account_id', $id, $account);
    }
    return $contact['records']->values;
  }
}

/**
 *  retreive contract information from an account
 */
function salesforce_contract_select($account = NULL, $params = array(), $defaults = true) {
  $salesforce = salesforce();
  $account = _salesforce_select_account($account);
  $admin = user_access('administer salesforce');

  // first we need to make sure we have an account_id to work with
  if (!$account->salesforce['account_id']) {
    $result = salesforce_account_select($account);
    $account->salesforce['account_id'] = $result['AccountId'];
    if (!$account->salesforce['account_id']) return array('error' => 'NO_ACCOUNT_ID_FOUND');
  }

  // merge default fields it required
  if ($defaults) {
    $default = array('Id', 'Status', 'ContractTerm', 'ContractNumber', 'ActivatedDate', 'StartDate', 'EndDate');
    $params = array_merge($default, $params);
  }
  
  // query salesforce for contract ids that this user has
  $result = $salesforce->query("SELECT id FROM contract WHERE accountId = '". $account->salesforce['account_id'] ."'");
  if (is_array($result['records'])) {
    foreach ($result['records'] as $row) {
      $contracts[] = $salesforce->retrieve('id', "contract", array($account->salesforce['account_id'], $account->salesforce['account_id']));
    }
  } else {
    $contracts = $salesforce->retrieve(implode(', ', $params), "contract", array($result['records']->id, $result['records']->id));
  }

  return $contracts;
}
