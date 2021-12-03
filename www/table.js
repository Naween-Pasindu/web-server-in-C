var
    filters = {
        user: null,
        status: null,
        milestone: null,
        priority: null,
        tags: null
    };

function updateFilters() {
    $('.task-list-row').hide().filter(function () {
        var
            self = $(this),
            result = true; // not guilty until proven guilty

        Object.keys(filters).forEach(function (filter) {
            if (filters[filter] && (filters[filter] != 'None') && (filters[filter] != 'Any')) {
                result = result && filters[filter] === self.data(filter);
            }
        });

        return result;
    }).show();
}

function changeFilter(filterName) {
    filters[filterName] = this.value;
    updateFilters();
}

// Assigned User Dropdown Filter
$('#assigned-user-filter').on('change', function () {
    changeFilter.call(this, 'user');
});

// Task Status Dropdown Filter
$('#status-filter').on('change', function () {
    changeFilter.call(this, 'status');
});

// Task Milestone Dropdown Filter
$('#milestone-filter').on('change', function () {
    changeFilter.call(this, 'milestone');
});

// Task Priority Dropdown Filter
$('#priority-filter').on('change', function () {
    changeFilter.call(this, 'priority');
});

// Task Tags Dropdown Filter
$('#tags-filter').on('change', function () {
    changeFilter.call(this, 'tags');
});


// text input filter
var $rows = $('#task-list-tbl tr');
$('#search').keyup(function() {
    var val = $.trim($(this).val()).replace(/ +/g, ' ').toLowerCase();
    
    $rows.show().filter(function() {
        var text = $(this).text().replace(/\s+/g, ' ').toLowerCase();
        return !~text.indexOf(val);
    }).hide();
});