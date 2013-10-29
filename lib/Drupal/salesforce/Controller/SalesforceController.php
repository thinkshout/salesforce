<?php

/**
 * @file
 * Contains \Drupal\salesforce\Controller\SalesforceController.
 */

namespace Drupal\salesforce\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\RedirectResponse;
use Symfony\Component\HttpKernel\Exception\AccessDeniedHttpException;

/**
 *
 */
class SalesforceController extends ControllerBase {

  /**
   * Callback for the oauth redirect URI.
   *
   * Exchanges an authorization code for an access token.
   */
  public function oauthCallback() {
    // If no code is provided, return access denied.
    if (!isset($_GET['code'])) {
      throw new AccessDeniedHttpException();
    }
    $salesforce = salesforce_get_api();
    $salesforce->requestToken($_GET['code']);

    salesforce_set_message('Salesforce OAUTH2 authorization successful.');

    return new RedirectResponse(url('admin/config/salesforce/authorize', array('absolute' => TRUE)));
  }
}
