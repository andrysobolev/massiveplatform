<?php
/**
 * @file tm_users_external_signin.pages
 * Page callbacks for tm_users_external_signin module.
 * This is heavily based on the twitter_signin module (http://drupal.org/project/twitter)
 * 
 * @author Daniel da Silva (daniel.silva@flipside.org)
 */


/**
 * Submit handler for Twitter signin.
 * The users go to this url to signin with twitter.
 */
function tm_users_external_signin_oauth() {
  $key = variable_get('tm_twitter_consumer_key', '');
  $secret = variable_get('tm_twitter_consumer_secret', '');
  $twitter = new Twitter($key, $secret);
  $token = $twitter->get_request_token();

  $_SESSION['twitter_oauth']['token'] = $token;
  $_SESSION['twitter_oauth']['destination'] = $_SERVER['HTTP_REFERER'];
  $_SESSION['twitter_oauth']['signin'] = TRUE;

  $url = $twitter->get_authenticate_url($token);
  drupal_goto($url);
}

/**
 * Callback handler for Twitter signin.
 * Either logs in the user or redirects to complete registration.
 */
function tm_users_external_signin_oauth_callback() {
  $key = variable_get('tm_twitter_consumer_key', '');
  $secret = variable_get('tm_twitter_consumer_secret', '');

  // Is the user requesting twitter integration?
  if (isset($_SESSION['twitter_oauth'])) {
    $twitter_oauth = $_SESSION['twitter_oauth'];
    unset($_SESSION['twitter_oauth']);
  }
  else {
    watchdog('tm_users_external_signin', 'Access to twitter callback without twitter_oauth', array(), WATCHDOG_ERROR);
    drupal_goto('<front>');
  }
  
  // Check is all the needed variables are present.
  if (isset($twitter_oauth['token'])) {
    $token = $twitter_oauth['token'];
    $response_oauth_token = $_GET['oauth_token'];
    $response_oauth_verifier = $_GET['oauth_verifier'];
    
    if (!is_array($token) || !$key || !$secret) {
      watchdog('tm_users_external_signin', 'Invalid Twitter OAuth request.', array(), WATCHDOG_ERROR);
      drupal_set_message(t('An error occurred. Please try again.'), 'error');
      drupal_goto('user/register');
    }
    if ($token['oauth_token'] != $response_oauth_token) {
      watchdog('tm_users_external_signin', 'Invalid Twitter OAuth request.', array(), WATCHDOG_ERROR);
      drupal_set_message(t('An error occurred. Please try again.'), 'error');
      drupal_goto('user/register');
    }
  }
  else {
    watchdog('tm_users_external_signin', 'Invalid Twitter OAuth request.', array(), WATCHDOG_ERROR);
    drupal_set_message(t('An error occurred. Please try again.'), 'error');
    drupal_goto('user/register');
  }
  
  // New request.
  $twitter = new Twitter($key, $secret, $token['oauth_token'], $token['oauth_token_secret']);
  
  // Make a request to exchange the give oauth_verifier for the user's tokens.
  $user_token = $twitter->get_access_token($response_oauth_verifier);
  if (!$user_token) {
    // No need fot a watchdog. Twitter lob will take care of that.
    drupal_set_message(t('An error occurred. Please try again.'), 'error');
    drupal_goto('user/register');
  }
  
  // Check if the user is in the database.
  $uid = db_query("SELECT uid
    FROM {tm_twitter_account}
    WHERE twitter_uid = :twitter_uid",
    array(':twitter_uid' => $user_token['user_id']))->fetchField();

  global $user;

  if ($uid) {
    // The user is already registered. Login the user.
    $account = user_load($uid);
    if ($account) {
      
      if ($account->status == 0) {
        drupal_set_message(t('The user has not been activated or is blocked.'), 'error');
        drupal_goto('user');
      }
      
      $user = $account;
      user_login_finalize();
      // TODO: Better redirection.
      drupal_goto('user');
    }
    else {
      watchdog('tm_users_external_signin', 'User with uid !uid found in the tm_twitter_account table does not correspond to a valid drupal user', array(':uid' => $uid ), WATCHDOG_ERROR);
      drupal_set_message(t('An error occurred. Please try again.'), 'error');
      drupal_goto('user');
    }
  }
  else {
    // A twitter account was not found.
    // Get all the data from twitter.
    $twitter_account = $twitter->get('users/show', array('user_id' => $user_token['user_id']));
    
    if ($user->uid) {
      // There is a logged user.
      // He's trying to link his account.
      
      // Be sure that this user doesn't have a linked account already.
      $twitter_uid = db_query("SELECT twitter_uid
        FROM {tm_twitter_account}
        WHERE uid = :uid",
        array(':uid' => $user->uid))->fetchField();
      
      if ($twitter_uid) {
        drupal_set_message(t('There\'s already a linked twitter account'), 'error');
      }
      else {
        $twitter_account['drupal_uid'] = $user->uid;
        tm_twitter_account_create($twitter_account, $user_token);
        drupal_set_message(t('Account successfully linked.'));
      }
      
      drupal_goto('user/' . $user->uid . '/edit/tm_twitter');
      
    }
    else {
      // No user logged in.
      // The user needs to be registered.
      $_SESSION['twitter_account']['account'] = $twitter_account;
      $_SESSION['twitter_account']['token'] = $user_token;
      
      // Redirect to finish registration.
      drupal_goto('tm_twitter/register');
    }
  }
}

/**
 * Form handler for tm_twitter/register.
 */
function tm_users_external_signin_register_form($form, &$form_state) {
  // If the user didn't come from the twitter callback redirect.
  if (!isset($_SESSION['twitter_account'])) {
    drupal_goto('<front>');
  }
  
  if (empty($_SESSION['twitter_account']['account']) || empty($_SESSION['twitter_account']['token'])) {
    watchdog('tm_users_external_signin', 'Twitter did not return proper values.', array(), WATCHDOG_ERROR);
    drupal_set_message(t('An error occurred. Please try again.'), 'error');
    drupal_goto('user/register');
  }
  
  // Shortcut
  $twitter_account = $_SESSION['twitter_account']['account'];
  
  // Temp:
  // TODO: Set a proper message (Maybe description field)
  drupal_set_message('Twitter doesn\'t provide an email. Please provide one.', 'status', FALSE);
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#default_value' => $twitter_account['screen_name'],
    '#title' => 'Username',
    '#size' => '30',
    '#required' => TRUE
  );  
  
  $form['mail'] = array(
    '#type' => 'textfield',
    '#title' => 'Email address',
    '#size' => '30',
    '#required' => TRUE,
    '#attributes' =>array('placeholder' => t('E-mail address'))
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Finish Registration',
  );
  
  $form['#submit'][] = 'tm_users_external_signin_register_form_submit';
  
  return $form;
}

/**
 * Form validation callback.
 */
function tm_users_external_signin_register_form_validate($form, &$form_state) {
  $valid_email = $form_state['values']['mail'];
  if (!valid_email_address($valid_email)) {
    form_set_error('mail', 'Sorry. Your email address,' . $valid_email . ', is not valid. Please submit a valid E-mail address.');
  }
}

/**
 * Form submit callback.
 * Simulate call the system register form and let logintoboggan do its
 * magic.
 */
function tm_users_external_signin_register_form_submit($form, &$form_state) {
  // User name:
  $name = $form_state['values']['name'];
  // User email:
  $email = $form_state['values']['mail'];
  // User password (generated):
  $pwd = user_password(8);
  
  // Reset the form_state for the user_reguster form.
  $form_state_user = array();
  $form_state_user['values']['name'] = $name;
  $form_state_user['values']['mail'] = $email;
  $form_state_user['values']['pass']['pass1'] = $pwd;
  $form_state_user['values']['pass']['pass2'] = $pwd;
  $form_state_user['values']['op'] = t('Save');
  
  // When submitting a form programmatically, drupal doesn't set a triggering
  // element. Apparently the file field has a problem with this.
  // Setting the triggering element to NULL does the trick but an explanation/
  // better solution would be more than welcome.
  $form_state_user['triggering_element']['#parents'][0] = NULL;
  
  drupal_form_submit('user_register_form', $form_state_user);
  
  // Check if the user registration yielded any errors.
  $errors = form_get_errors();
  
  if (!$errors) {
    // No errors. Get the user id using the email (unique key).
    $uid = db_query('SELECT uid FROM {users} WHERE mail = :mail', array(':mail' => $form_state['values']['mail']))->fetchField();
    
    // Link the twitter account.
    $account = $_SESSION['twitter_account']['account'];
    $token = $_SESSION['twitter_account']['token'];
    unset($_SESSION['twitter_account']);
    
    $account['drupal_uid'] = $uid;
    tm_twitter_account_create($account, $token);
    
    // The user is logged in by the logintoboggan module.
    
    // Use the redirect set by the logintoboggan module.
    $form_state['redirect'] = $form_state_user['redirect'];
  }
}

/**
 * Callback for user/%user/edit/tm_twitter
 */
function tm_users_external_signin_manage_account($account) {
  $twitter_data = tm_twitter_account_load($account->uid);
  $output = '';
  
  if ($twitter_data) {
    $form = drupal_get_form('tm_users_external_signin_manage_account_unlink_form', $account, $twitter_data);
    $output .= drupal_render($form);
  }
  else {
    $output .= 'Your account is not linked with twitter.</br>';
    $output .= 'Click here to ' . l('link your account', 'tm_twitter/oauth');
  }

  return $output;
}

/**
 * Unlink account form.
 */
function tm_users_external_signin_manage_account_unlink_form($form, &$form_state, $user_account, $twitter_account) {
  $form['user_uid'] = array(
    '#type' => 'hidden',
    '#value' => $user_account->uid,
  );
  // Temp.
  $form['description'] = array(
    '#markup' => t('This account is linked with twitter user %uname<br/>', array('%uname' => '@' . $twitter_account['screen_name']))
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Unlink account')
  );
  return $form;
}

/**
 * Unlink account form submit handler.
 */
function tm_users_external_signin_manage_account_unlink_form_submit($form, &$form_state) {
  if (tm_twitter_account_delete($form_state['values']['user_uid'])) {
    drupal_set_message(t('Twitter account successfully unlinked.'));
  }
  else {
    drupal_set_message(t('Error unlinking twitter account.'), 'error');
  }
}