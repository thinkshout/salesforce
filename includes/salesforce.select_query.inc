<?php
/**
 * @file
 * Class representing a Salesforce SELECT SOQL query.
 */

class SalesforceSelectQuery {

  public $fields = array();
  public $order = array();
  public $objectType;
  public $limit;
  protected $where = array();

  /**
   * Constructor which sets the query object type.
   *
   * @param string $object_type
   *   Salesforce object type to query.
   */
  public function __construct($object_type = '') {
    $this->objectType = $object_type;
  }

  /**
   * Add a condition to the query.
   *
   * @param string $name
   *   Field name.
   * @param mixed $value
   *   Condition value. If an array, it will be split into quote enclosed
   *   strings separated by commas inside of parenthesis. Note that the caller
   *   must enclose the value in quotes as needed by the SF API.
   * @param string $operator
   *   Conditional operator. One of '=', '!=', '<', '>', 'LIKE, 'IN', 'NOT IN'.
   */
  public function addCondition($name, $value, $operator = '=') {
    $clause = $value;
    if (is_array($value)) {
      $clause = "('" . implode("','", $value) . "')";

      // Set operator to IN if wasn't already changed from the default.
      if ($operator == '=') {
        $operator = 'IN';
      }
    }

    $this->where[] = "$name+$operator+$clause";
  }

  /**
   * Implements PHP's magic toString().
   *
   * Function to convert the query to a string to pass to the SF API.
   *
   * @return string
   *   SOQL query ready to be executed the SF API.
   */
   // @codingStandardsIgnoreStart
  public function __toString() {

    $query = 'SELECT+';
    $query .= implode(',+', $this->fields);
    $query .= "+FROM+" . $this->objectType;

    if (count($this->where) > 0) {
      $query .= '+WHERE+' . implode('+AND+', $this->where);
    }

    if ($this->order) {
      $query .= "+ORDER BY+";
      $fields = array();
      foreach ($this->order as $field => $direction) {
        $fields[] = $field . ' ' . $direction;
      }
      $query .= implode(',+', $fields);
    }

    if ($this->limit) {
      $query .= "+LIMIT+" . (int) $this->limit;
    }

    return $query;
  }
  // @codingStandardsIgnoreEnd
}
