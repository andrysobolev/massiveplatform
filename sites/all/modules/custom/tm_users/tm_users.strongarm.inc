<?php
/**
 * @file
 * tm_users.strongarm.inc
 */

/**
 * Implements hook_strongarm().
 */
function tm_users_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'field_bundle_settings_user__user';
  $strongarm->value = array(
    'view_modes' => array(
      'grid' => array(
        'custom_settings' => TRUE,
      ),
      'list' => array(
        'custom_settings' => TRUE,
      ),
      'full' => array(
        'custom_settings' => FALSE,
      ),
      'diff_standard' => array(
        'custom_settings' => FALSE,
      ),
      'token' => array(
        'custom_settings' => FALSE,
      ),
    ),
    'extra_fields' => array(
      'form' => array(
        'masquerade' => array(
          'weight' => '50',
        ),
        'account' => array(
          'weight' => '-10',
        ),
        'timezone' => array(
          'weight' => '6',
        ),
      ),
      'display' => array(
        'masquerade' => array(
          'default' => array(
            'weight' => '7',
            'visible' => FALSE,
          ),
          'grid' => array(
            'weight' => '8',
            'visible' => FALSE,
          ),
          'list' => array(
            'weight' => '50',
            'visible' => TRUE,
          ),
        ),
        'summary' => array(
          'default' => array(
            'weight' => '6',
            'visible' => FALSE,
          ),
          'grid' => array(
            'weight' => '7',
            'visible' => FALSE,
          ),
          'list' => array(
            'weight' => '5',
            'visible' => TRUE,
          ),
        ),
      ),
    ),
  );
  $export['field_bundle_settings_user__user'] = $strongarm;

  return $export;
}