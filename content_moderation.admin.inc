<?php
// $Id$

/**
 * Form builder function for module settings.
 */
function content_moderation_admin_settings() {
	$form['content_moderation_pending_count'] = array(
    '#title' => t('Number of pending revisions to be shown in content_moderation block'),
    '#type' => 'textfield',
		'#size' => 3,
		'#maxlength' => 3,
    '#description' => t('Define how many pending revisions shall be visible in content_moderation block. This number will restrict the shown revisions cutting out oldest ones first. By setting this to 1 you\'ll only ever see the newest pending revision.'),
		'#default_value' => variable_get('content_moderation_pending_count', 5),
  );
  $form['content_moderation_moderate_latest_only'] = array(
    '#title' => t('Edit stop on revision-moderation'),
    '#type' => 'checkbox',
    '#description' => t('When you enable this, you will only be able to<br> - Change the state of the latest revision of the dokument<br> - Edit the dokument only if the latest state has the status none<br>Please refer to the documentation for further details<br><strong>Be aware enabling this feature if you have already moderated revisions which are not the latest!</strong><br> <strong>Only disable this feature if you exactly know what you are going to expect!</strong>'),
    '#default_value' => variable_get('content_moderation_moderate_latest_only', 1),
  );
  $form['content_moderation_force_publish'] = array(
    '#title' => t('Force publishing on new nodes'),
    '#type' => 'checkbox',
		'#description' => t('The intended default value is checked (feature by design).<br/>However, it is then impossible to first create a(n unpublished) draft. By unchecking this feature content types\' publishing options may be set to "unpublished". When a draft goes "live" (transition into moderation state "live") for the first time the node will be published automatically. After that only users with the right "administer nodes" are able to unpublish it again (making it a draft).<br/>'),
		'#default_value' => variable_get('content_moderation_force_publish', 1),
  );



	return system_settings_form($form);
}


/**
 * Form builder function for module settings.
 * Defining states
 */
function content_moderation_admin_settings_states() {

	$form['content_moderation_states'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content moderation states'),
    '#description' => t('Use to define new states or to delete existing ones. States have to be defined before corresponding transitions can be set up.')
  );

  $form['content_moderation_states']['delete_states'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Delete existing states'),
    '#default_value' => array(),
    '#options' => _content_moderation_admin_states_names(),
    '#description' => t('Please note: when deleting an existing state all transitions involving this state will also be deleted.')
  );

  $form['content_moderation_states']['new_state_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Create new state'),
    '#size' => 60,
    '#maxlength' => 255,
    '#description' => t('Enter name of new state.')
  );

  $form['content_moderation_states']['new_state_desc'] = array(
    '#type' => 'textarea',
    '#description' => t('Enter an optional description of new state.')
  );

  $form['content_moderation_states']['submit_states'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

	return $form;
}


/**
 * Form builder function for module settings.
 * Defining transitions
 */
function content_moderation_admin_settings_transitions() {
  $form['content_moderation_transitions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content moderation transitions'),
    '#description' => t('Use to define new transitions or to delete existing ones. Transitions may be set up with existing states only.')
  );

  $form['content_moderation_transitions']['delete_transitions'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Delete existing transitions'),
    '#default_value' => array(),
    '#options' => _content_moderation_admin_transitions(),
    '#description' => t('Please note: deleting an existing transition will not have any impact on states involved.')
  );

  $form['content_moderation_transitions']['new_transition_from'] = array(
    '#type' => 'select',
    '#title' => t('New transition from'),
    '#default_value' => array(),
    '#options' => array_merge(array(0 => t('Choose a state')),_content_moderation_admin_states_names()),
  );

  $form['content_moderation_transitions']['new_transition_to'] = array(
    '#type' => 'select',
    '#title' => t('to'),
    '#default_value' => array(),
    '#options' => array_merge(array(0 => t('Choose a state')),_content_moderation_admin_states_names()),
  );

  $form['content_moderation_transitions']['submit_transitions'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );

  return $form;
}

/**
 * Get all states possible, but only name and description.
 */
function _content_moderation_admin_states_names() {
  $states = array();
  foreach (content_moderation_states() as $name => $state) {
    $states[$name] = $state->descr ? $state->descr : $state->name;
  }
  return $states;
}

/*
 * Get all transitions possible
 */
function _content_moderation_admin_transitions() {
  $result = db_query('select * from {content_moderation_transition}');
  //TODO: cache this
  $transitions = array();
  while ($transition = db_fetch_object($result)) {
    $transinfo = $transition->from_name.'---'.$transition->to_name;
    $transitions[$transinfo] = $transition->from_name . ' -> ' . $transition->to_name;
  }
  return $transitions;
}

function content_moderation_admin_settings_states_submit($form, &$form_state) {
  $form_values = $form_state['values'];
	foreach ( $form_values['delete_states'] AS $state => $action ) {
		if ( is_string($action) ) {
			$q = ' DELETE FROM {content_moderation_states} WHERE name = "%s" ';
			$r = db_query($q, array($state));
			$q = ' DELETE FROM {content_moderation_transition} WHERE from_name = "%s" OR from_name = "%s" ';
			$r = db_query($q, array($state, $state));
		}
	}
	if ( isset($form_values['new_state_name']) && strlen($form_values['new_state_name']) > 0 ) {
    $new_state = (object) null;
    $new_state->name = $form_values['new_state_name'];
		if ( isset($form_values['new_state_desc']) && strlen($form_values['new_state_desc']) > 0 ) {
      $new_state->desc = $form_values['new_state_desc'];
		}
    drupal_write_record('content_moderation_states', $new_state);
	}
	drupal_set_message(t('Your settings have been saved.'));
}

function content_moderation_admin_settings_transitions_submit($form, &$form_state)
{
	$form_values = $form_state['values'];
	foreach ( $form_values['delete_transitions'] AS $transition => $action ) {
		if ( is_string($action) ) {
			$transinfo = explode('---', $transition);
			$q = ' DELETE FROM {content_moderation_transition} WHERE from_name = "%s" AND to_name = "%s" ';
			$r = db_query($q, array($transinfo[0], $transinfo[1]));
		}
	}
	if ( $form_values['new_transition_from'] != '0' && $form_values['new_transition_to'] != '0' ) {
    $new_transition = (object) null;
    $new_transition->from_name = $form_values['new_transition_from'];
    $new_transition->to_name = $form_values['new_transition_to'];
    $new_transition->ntype = 'all';
		drupal_write_record('content_moderation_transition', $new_transition);
	}
  drupal_set_message(t('Your settings have been saved.'));
}