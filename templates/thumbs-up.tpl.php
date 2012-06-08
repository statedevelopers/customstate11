<?php

/**
 * @file
 * Rate widget theme
 *
 * $info = A string phrase containing the vote count. 
 * $results['count'] = Integer for the number of votes. 
 * 
 */
(!empty($results['user_vote'])) ? $user_vote = $results['user_vote'] : $user_vote = FALSE;

if ($user_vote) {
  $up_button = '<div'
    . ' class="rate-marker rate-thumbs-up-btn-up"'
    . ' id="rate-button-' . $id . '" '
    . '>'
    . 'Like'
    . '</div>';
} else {
  $up_button = '<a'
    . ' class="rate-button rate-thumbs-up-btn-up"'
    . ' id="rate-button-' . $id . '" rel="nofollow" '
    . 'href="' . $links[0]['href'] . '">'
    . 'Like'
    . '</a>';
}
print $up_button;

switch ($results['count']) {
  case 0: 
    break;
  case 1: 
    print '<div class="rate-info">' . $results['count'] . '&nbsp;like</div>';
    break;
  default: 
    print '<div class="rate-info">' . $results['count'] . '&nbsp;likes</div>';
    break;
}

if ($display_options['description']) {
  print '<div class="rate-description">' . $display_options['description'] . '</div>';
}
