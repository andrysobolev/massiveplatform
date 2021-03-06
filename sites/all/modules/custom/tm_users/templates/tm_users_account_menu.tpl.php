<?php global $user;
// Only a loaded user has values for the fields.
$loaded = user_load($user->uid);
if (empty($loaded->field_avatar)) {
  // It is not possible to get an imagefield default value
  // using the standard default function. Load the file instead.
  $field = field_info_field('field_avatar');
  $default = file_load($field['settings']['default_image']);
  $img_uri = $default->uri;
}
else {
  $img_uri = $loaded->field_avatar[LANGUAGE_NONE][0]['uri'];
}

$image = theme('image_style', array(
  'style_name' => 'avatar',
  'path' => $img_uri,
  'alt' => 'user image',
  'title' => 'The user image',
));
?>

<h2>
  <a class="toggle" href="#account-menu-blk" data-dropd-toggle>
    <span class="hide"><?= t('Account'); ?></span>
    <?php if ($user->uid) : ?>
    <span class="avatar"><?php print $image; ?></span>
    <?php endif; ?>
  </a>
</h2>
<div id="account-menu-blk" class="inner dropd dropd-right" data-dropd>

  <?php if ($user->uid) : ?>
    <ul class="dropd-menu">
      <li>
        <div class="media-obj">
          <a href="<?php print url('user/' . $loaded->uid . '/edit', array('fragment' => 'user-profile-options')); ?>">
            <div class="media-fig">
              <span class="avatar"><?php print $image; ?></span>
            </div>
            <div class="media-bd">
              <strong><?php print check_plain($loaded->realname); ?></strong>
              <?php print t('Edit profile'); ?>
              <!--<?php if (!in_array("approved user", $user->roles)) { ?><br><i>Pending Approval</i><?php } ?>-->
            </div>
          </a>
        </div>
      </li>
    </ul>
    <ul class="dropd-menu">
      <li><?php print l(t('Public profile'), 'user/' . $loaded->uid); ?></li>
      <li><?php print l(t('Account settings'), 'user/' . $loaded->uid . '/edit', array('fragment' => 'user-account-options')); ?></li>
      <li><?php print l(t('Notification settings'), 'user/' . $loaded->uid . '/edit', array('fragment' => 'user-notifications-options')); ?></li>
      <li><?php print l(t('Invite members'), 'invite'); ?></li>
<?php
$twitter_data = tm_twitter_account_load($loaded->uid);
if (!$twitter_data) { 
?>
      <li><?php print l(t('Connect with Twitter'), 'tm_twitter/oauth'); ?></li>
<?php
} // end if
?>

    </ul>
  <?php if (in_array("approved user", $loaded->roles)) : ?>
    <ul class="dropd-menu">
      <?php print tm_users_companies($loaded->uid); ?>
      <!--<li><?php print l(t('My companies'), 'user/' . $loaded->uid . '/companies', array('fragment' => 'user-notifications-options')); ?></li>-->
      <li><?php print l(t('Add company'), 'node/add/organization'); ?></li>
    </ul>

  <?php endif; ?>
    <!-- Link by title to any chapters you are a chapter leader of -->
    <?php if (in_array("chapter leader", $loaded->roles)) : ?>
      <ul class="dropd-menu">
        <?php print tm_users_chapters($loaded->uid); ?>
      </ul>
  <?php endif; ?>

  <?php if (in_array("moderator", $loaded->roles)) : ?>
      <ul class="dropd-menu">
        <li><?php print l(t('Add chapter'), 'node/add/organization'); ?></li>
        <li><?php print l(t('Unapproved users'), 'admin/config/tm/unapproved-users'); ?></li>
      </ul>
  <?php endif; ?>

    <ul class="dropd-menu">
      <li><?php print l(t('Sign out'), 'user/logout'); ?></li>
    </ul>

  <?php else : ?>

    <h3 class="menu-blk-title">Sign in</h3>
    <p><?php print l(t('Twitter'), 'tm_twitter/oauth', array('attributes' => array('class' => 'twitter-login'))); ?></p>
    <i class="or">or</i>

    <?php $login_form = drupal_get_form('user_login_block'); ?>
    <?php print render($login_form); ?>

    <?php if (variable_get('user_register', 1)) : ?>
      <h3 class="menu-blk-title">New to Travel Massive?</h3>
      <p><?php print l(t('Sign up now'), 'user/register', array('attributes' => array('title' => 'Create account', 'class' => array('cta-inline')))); ?></p>
    <?php endif; ?>

  <?php endif; ?>

</div>
