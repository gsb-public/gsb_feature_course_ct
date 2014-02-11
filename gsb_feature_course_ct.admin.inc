<?php

/**
 * Functions related to Course Filtering 
 */

/**
 * Validate the subject_code argument 
 */
function gsb_feature_course_ct_subject_code_load($subject_code = null) {
  if ($subject_code != null) {
    $subject = '';
    $code = '';
    list($subject, $code) = explode('-',$subject_code);
    // Check if subject is valid
    $subjects = _gsb_feature_course_ct_get_subjects();
    if (!in_array(str_replace('ALL', 'All', $subject),$subjects)) {
      drupal_set_message(t('Page not found. Subject is invalid.'), 'error', FALSE);
      drupal_goto($path = 'admin/config/gsb/course-filters');
      return FALSE;
    }    
    // Check if code is valid - 3 digits in length 
    if (!is_numeric($code)) {
      drupal_set_message(t('Page not found. Code is invalid.'), 'error', FALSE);
      drupal_goto($path = 'admin/config/gsb/course-filters');      
      return FALSE;
    } 
    else if (strlen($code) != 3)  {
      drupal_set_message(t('Page not found. Code is invalid.'), 'error', FALSE);
      drupal_goto($path = 'admin/config/gsb/course-filters');            
      return FALSE;
    }
    return array(
      'subject' => $subject,
      'code' => $code,
    );     
  }
  return array(
    'subject' => '',
    'code' => '',
  );  
}

/**
 * List all Course Filters in table/operations form 
 */
function gsb_feature_course_ct_list_filters($form, &$form_state) {

  $form['addlink'] = array(
    '#type' => 'link',
    '#title' => t('Add filter'),
    '#href' => 'admin/config/gsb/course-filters/add',
  );

  $course_filters = _gsb_feature_course_ct_get_course_filters();

  if (!empty($course_filters)) {

    $rows = array();

    ksort($course_filters);
    _gsb_feature_course_ct_move_all_first($course_filters);

    foreach ($course_filters as $key => $codes) {

      sort($codes);

      foreach($codes as $code) {

        $row = array();

        $row['data'][] = $key;
        $row['data'][] = $code;

        $operations = array();
        $operations['edit'] = array(
          'title' => t('Edit'),
          'href' => 'admin/config/gsb/course-filters/' . $key .'-' . $code . '/edit',
        );
        $operations['delete'] = array(
          'title' => t('Delete'),
          'href' => 'admin/config/gsb/course-filters/' . $key .'-' . $code . '/delete',
        );
        $row['data']['operations'] = array(
          'data' => array(
            '#theme' => 'links',
            '#links' => $operations,
            '#attributes' => array('class' => array('links', 'inline')),
          ),
        );  

        $rows[$key .'-'.$code] = $row;

      }

    }

    $form['table'] = array(
      '#theme' => 'table',
      '#header' => array(
        t('Course Subject'),
        t('Course Code'),
        t('Operations'),
      ),
      '#rows' => $rows,
    );  

  }

  return $form; 
}

/**
 * Edit the Course Filter.
 */
function gsb_feature_course_ct_edit_filter($form, $form_state, $subject_code = array()) {

  $subject = null;
  $code = null;
  if (!empty($subject_code)) {
    $subject = $subject_code['subject'];
    $code = $subject_code['code'];
  }

  $form = _gsb_feature_course_ct_get_basic_form();

  if ($subject != null) {
    $form['course_subject']['#default_value'] = $subject;
    $form['course_subject']['#disabled'] = TRUE;      
  }
  
  if ($code != null) {
    $form['course_code']['#default_value'] = $code;    
  }

  if ($subject_code != null) {
    $form['subject_code'] = array(
      '#type' => 'hidden',
      '#value' => implode('-', $subject_code),
    );       
  }  

  $form['#validate'][] = 'gsb_feature_course_ct_edit_filter_validate';   
  $form['#submit'][] = 'gsb_feature_course_ct_edit_filter_submit';    

  return system_settings_form($form, $form_state);
}  

/**
 * Validate the Course Filter Edit form.
 */
function gsb_feature_course_ct_edit_filter_validate($form, &$form_state) {
  $code = $form_state['values']['course_code'];
  // Check if a code has been entered
  if ($code == '') {
    form_set_error('course_code', t('Please set a course code for the filter.'));
  }
  // Check if code is valid - 3 digits in length 
  if (!is_numeric($code)) {
    form_set_error('course_code', t('Course code for the filter must be numeric.'));
  } 
  else if (strlen($code) != 3)  {
    form_set_error('course_code', t('Course code for the filter must be 3 digits in length.'));
  }  
}

/**
 * Save the Course Filter.
 */
function gsb_feature_course_ct_edit_filter_submit($form, &$form_state) {

  $subject = $form_state['values']['course_subject'];
  $code = $form_state['values']['course_code'];

  if (!empty($form_state['values']['subject_code'])) {
    // get the subject and previous code
    $subject_code = $form_state['values']['subject_code'];
    list($subject, $old_code) = explode('-',$subject_code);
    // delete the old code
    _gsb_feature_course_ct_delete_filter_code($subject, $old_code);
  }

  // add the new code
  _gsb_feature_course_ct_add_filter_code($subject, $code);

  $form_state['redirect'] = array('admin/config/gsb/course-filters');

}

/**
 * Delete the Course Filter.
 */                                         
function gsb_feature_course_ct_delete_filter($form, $form_state, $subject_code = array()) {

  $question = t('Are you sure you want to delete this filter?');
  $destination = 'admin/config/gsb/course-filters';

  if (!empty($subject_code)) {
    $form['subject_code'] = array(
      '#type' => 'value',
      '#value' => $subject_code,
    );            
  }

  return confirm_form($form, NULL, $destination, $question, t('Delete'), t('Cancel'));
}  

function gsb_feature_course_ct_delete_filter_submit($form, &$form_state) {

  if (!empty($form_state['values']['subject_code'])) {
    // delete the code
    _gsb_feature_course_ct_delete_filter_code(
      $form_state['values']['subject_code']['subject'],
      $form_state['values']['subject_code']['code']
    );
  }

  $form_state['redirect'] = array('admin/config/gsb/course-filters');

}  

/**
 * Add the filter code 
 */
function _gsb_feature_course_ct_add_filter_code($subject, $code) {

  $course_filters = _gsb_feature_course_ct_get_course_filters();

  if(!empty($course_filters[$subject])) {
    $course_filters[$subject] = array_unique (array_merge($course_filters[$subject], array($code)));
  }
  else {
    $course_filters[$subject] = array($code);
  }

  _gsb_feature_course_ct_set_course_filters($course_filters);
}

/**
 * Delete the filter code 
 */
function _gsb_feature_course_ct_delete_filter_code($subject, $code) {

  $course_filters = _gsb_feature_course_ct_get_course_filters();

  if (!empty($course_filters[$subject])) {
    $codes = $course_filters[$subject];
    foreach($course_filters[$subject] as $key => $value) {
      if ($value == $code) {
        unset($codes[$key]);
        break;
      }    
    }  
    $course_filters[$subject] = $codes;
  }

  _gsb_feature_course_ct_set_course_filters($course_filters);
}  

/**
 * Create the basic fields for the edit form
 */
function _gsb_feature_course_ct_get_basic_form() {

  $form = array();

  $subjects = _gsb_feature_course_ct_get_subjects();

  $form['course_subject'] = array(
    '#type' => 'select',
    '#title' => t('Select Course Subject'),
    '#options' => $subjects,
    '#description' => t('Select which course subject that will be used in the filter.')
  );  

  $form['course_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Course Code'),
    '#required' => TRUE,
    '#description' => t('Set the course code that will be used in the filter.')
  );    

  return $form;
}  

/**
 * Modify the order of the listing 
 */
function _gsb_feature_course_ct_move_all_first(&$course_filters) {

  // We move the items marked with the 'all' subject to the top 
  // of the listed items, since these have global impact and 
  // therefore have more importance

  if (!empty($course_filters['ALL'])) {
    $all = $course_filters['ALL'];
    unset($course_filters['ALL']);
    $temp = array();
    $temp['ALL'] = $all;
    $course_filters = $temp + $course_filters;
  } 

}

/**
 * List of subjects
 */
function _gsb_feature_course_ct_get_subjects() {
  $subjects = array(
    'ALL' => 'All',
    'ACCT' => 'ACCT',
    'FINANCE' => 'FINANCE',
    'GSBGEN' => 'GSBGEN',
    'HRMGT' => 'HRMGT',
    'MGTECON' => 'MGTECON',    
    'MKTG' => 'MKTG',
    'OB' => 'OB',    
    'OIT' => 'OIT',
    'POLECON' => 'POLECON',
    'STRAMGT' => 'STRAMGT',
  );  
  return $subjects;  
}

function _gsb_feature_course_ct_get_course_filters() {
  return variable_get('gsb_course_ct_filters', array());
}

function _gsb_feature_course_ct_set_course_filters($value) {
  return variable_set('gsb_course_ct_filters', $value);
}



